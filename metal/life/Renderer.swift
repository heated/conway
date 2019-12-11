import Metal
import MetalKit

let size = rows * cols
let memUnit = MemoryLayout<UInt32>.stride
let memTotal = memUnit * Int(size)
let gensPerFrame = 350
let frameGensVariance = 6

let maxBuffersInFlight = 3
let ceil = Int((size - 1)/256 + 1)
let gridSize = MTLSizeMake(ceil, 1, 1)
let threadGroupSize = MTLSizeMake(256, 1, 1)

class Renderer: NSObject, MTKViewDelegate {
    public let device: MTLDevice
    let inFlightSemaphore = DispatchSemaphore(value: maxBuffersInFlight)
    let commandQueue: MTLCommandQueue
    let simState: MTLComputePipelineState
    var drawState: MTLRenderPipelineState
    var vertexBuffer: MTLBuffer
    var cellsBuffer: MTLBuffer
    var newCellsBuffer: MTLBuffer
    var sigRandom = true
    let vertexData: [Float] = [ /// full-screen quad from triangles
        -1, -1, /// lower left triangle
         1, -1,
        -1,  1,
         1,  1, /// upper right triangle
         1, -1,
        -1,  1,
    ]

    init?(metalKitView: MTKView) {
        self.device = metalKitView.device!
        self.commandQueue = self.device.makeCommandQueue()!
        
        let library = device.makeDefaultLibrary()!
        let vertexFunction = library.makeFunction(name: "vertexShader")
        let fragmentFunction = library.makeFunction(name: "fragmentShader")
        let nextGen = library.makeFunction(name: "nextGen")!
        
        let pipelineDescriptor = MTLRenderPipelineDescriptor()
        pipelineDescriptor.vertexFunction = vertexFunction
        pipelineDescriptor.fragmentFunction = fragmentFunction
        pipelineDescriptor.colorAttachments[0].pixelFormat = metalKitView.colorPixelFormat
        
        drawState = try! device.makeRenderPipelineState(descriptor: pipelineDescriptor)
        simState = try! device.makeComputePipelineState(function: nextGen)
        
        let dataSize = vertexData.count * MemoryLayout.size(ofValue: vertexData[0])
        vertexBuffer = device.makeBuffer(bytes: vertexData, length: dataSize, options: [])!
        cellsBuffer = device.makeBuffer(length: memTotal)!
        newCellsBuffer = device.makeBuffer(length: memTotal)!
        
        super.init()
        startEngine()
    }
    
    func startEngine() {
        randomizeCells()
        
        let _ = Timer.scheduledTimer(timeInterval: 0.7, target: self, selector: #selector(scheduleRandomization), userInfo: nil, repeats: true)
    }
    
    @objc func scheduleRandomization() {
        sigRandom = true
    }

    func randomizeCells() {
        let contents = cellsBuffer.contents()
        for index in 0..<size {
            let bytes = UInt32.random(in: 0..<UInt32.max)
            contents.storeBytes(of: bytes,
                                toByteOffset: Int(index) * memUnit,
                                as: UInt32.self)
        }
    }

    func simulate() {
        let buffer = commandQueue.makeCommandBuffer()!
        let encoder = buffer.makeComputeCommandEncoder()!

        encoder.setComputePipelineState(simState)
        let lowerBound = gensPerFrame - frameGensVariance
        let gens = Int.random(in: lowerBound..<gensPerFrame)
        for _ in 0..<gens {
            encoder.setBuffers([cellsBuffer, newCellsBuffer], offsets: [0, 0], range: 0..<2)
            encoder.dispatchThreadgroups(gridSize, threadsPerThreadgroup: threadGroupSize)
            swap(&cellsBuffer, &newCellsBuffer)
        }
        encoder.endEncoding()
        buffer.commit()
    }

    func draw(in view: MTKView) {
        /// Per frame updates hare
        _ = inFlightSemaphore.wait(timeout: DispatchTime.distantFuture)
        
        if let commandBuffer = commandQueue.makeCommandBuffer() {
            let semaphore = inFlightSemaphore
            commandBuffer.addCompletedHandler { (_ commandBuffer)-> Swift.Void in
                semaphore.signal()
            }
            
            if sigRandom {
                sigRandom = false
                randomizeCells()
            }
            
            simulate()
            
            /// Delay getting the currentRenderPassDescriptor until we absolutely need it to avoid
            ///   holding onto the drawable and blocking the display pipeline any longer than necessary
            let renderPassDescriptor = view.currentRenderPassDescriptor
            
            if let renderPassDescriptor = renderPassDescriptor {
                /// Final pass rendering code here
                if let renderEncoder = commandBuffer.makeRenderCommandEncoder(descriptor: renderPassDescriptor) {
                    renderEncoder.setRenderPipelineState(drawState)
                    renderEncoder.setVertexBuffer(vertexBuffer, offset: 0, index: 0)
                    renderEncoder.setFragmentBuffer(cellsBuffer, offset: 0, index: 0)
                    renderEncoder.drawPrimitives(type: MTLPrimitiveType.triangle, vertexStart: 0, vertexCount: 6)
                    renderEncoder.endEncoding()
                    
                    if let drawable = view.currentDrawable {
                        commandBuffer.present(drawable)
                    }
                }
            }
            
            commandBuffer.commit()
        }
    }

    func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize) {
        /// Respond to drawable size or orientation changes here
    }
}

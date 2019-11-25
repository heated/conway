import Metal
import MetalKit

let unit = MemoryLayout<UInt32>.stride
let size = rows * cols
let memSize = unit * Int(size)

// 32 hardware threads
let ceil = Int((size - 1)/256 + 1)
let gridSize = MTLSizeMake(ceil, 1, 1)
let threadGroupSize = MTLSizeMake(256, 1, 1)

class Renderer: NSObject, MTKViewDelegate {
    public let device: MTLDevice
    let commandQueue: MTLCommandQueue
    let simState: MTLComputePipelineState
    var drawState: MTLRenderPipelineState
    var vertexBuffer: MTLBuffer
    var cellsBuffer: MTLBuffer
    var newCellsBuffer: MTLBuffer
    let vertexData: [Float] = [
        -1, -1,
         1, -1,
        -1,  1,
         1,  1,
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
        cellsBuffer = device.makeBuffer(length: memSize)!
        newCellsBuffer = device.makeBuffer(length: memSize)!
        
        super.init()
        startEngine()
    }
    
    func startEngine() {
        randomizeCells()
        for _ in 0..<40 {
            loopNextGen()
        }
        
        let _ = Timer.scheduledTimer(timeInterval: 5, target: self, selector: #selector(randomizeCells), userInfo: nil, repeats: true)
    }

    @objc func randomizeCells() {
        let contents = cellsBuffer.contents()
        for index in 0..<size {
            let bytes = UInt32.random(in: 0..<UInt32.max)
            contents.storeBytes(of: bytes,
                                toByteOffset: Int(index) * unit,
                                as: UInt32.self);
        }
    }

    // takes up a slot in the commandqueue
    // don't call >~50x from outside
    func loopNextGen() {
        let buffer = commandQueue.makeCommandBuffer()!
        let encoder = buffer.makeComputeCommandEncoder()!
        buffer.addCompletedHandler { _ in
            self.loopNextGen()
        }

        encoder.setComputePipelineState(simState)
        encoder.setBuffers([cellsBuffer, newCellsBuffer], offsets: [0, 0], range: 0..<2)
        encoder.dispatchThreadgroups(gridSize, threadsPerThreadgroup: threadGroupSize)
        swap(&cellsBuffer, &newCellsBuffer)
        encoder.endEncoding()
        buffer.commit()
    }

    func draw(in view: MTKView) {
        /// Per frame updates hare
        if let commandBuffer = commandQueue.makeCommandBuffer() {
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

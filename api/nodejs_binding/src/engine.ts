import {BinaryDeserializer, BinarySerializer, ByteConst, MathC, Slice, Span, Token, Vector} from "./buffer_lib";
import * as path from "path"

type ArrayBufferOrFunction = ArrayBuffer | ((any: any) => any);

interface IEngineSharedLibrary {
    Finalize(): void;

    LoadDynamicLib(p_path: string): void;

    EntryPoint(...p_args: ArrayBufferOrFunction[]): ArrayBuffer;
};

export let engine: IEngineSharedLibrary = require("../../../../_install/build/Debug/NodeJsBinding") as IEngineSharedLibrary;
engine.LoadDynamicLib("../../../../cmake-build-debug/api/EngineFull.dll");


export enum EngineFunctionsKey {
    SpawnEngine = 0,
    DestroyEngine,
    MainLoopNonBlocking,
    FrameBefore,
    FrameAfter,
    DeltaTime,
    FrameCount,
    Node_CreateRoot,
    Node_Remove,
    Node_GetLocalPosition,
    Node_AddWorldRotation,
    Node_AddCamera,
    Node_AddMeshRenderer
};

export enum EngineLoopState {
    UNDEFINED = 0,
    ABORTED,
    IDLE,
    FRAME
};

export class EngineFuncKey {
    tok: Span;

    static allocate(p_type: EngineFunctionsKey): EngineFuncKey {
        let l_return = new EngineFuncKey();
        l_return.tok = Span.allocate(ByteConst.INT8);
        l_return.tok.to_slice().set_int8(p_type);
        return l_return;
    };
};


export class v3f {
    public x: number;
    public y: number;
    public z: number;

    public static build(p_x: number, p_y: number, p_z: number): v3f {
        let l_return = new v3f();
        l_return.x = p_x;
        l_return.y = p_y;
        l_return.z = p_z;
        return l_return;
    };

    public serialize(in_out_vector: Vector) {
        BinarySerializer.float32(in_out_vector, this.x);
        BinarySerializer.float32(in_out_vector, this.y);
        BinarySerializer.float32(in_out_vector, this.z);
    };

    public static deserialize(in_slice: Slice): v3f {
        let l_return: v3f = new v3f();
        l_return.x = BinaryDeserializer.float32(in_slice);
        l_return.y = BinaryDeserializer.float32(in_slice);
        l_return.z = BinaryDeserializer.float32(in_slice);
        return l_return;
    };

    public equals(p_other: v3f): boolean {
        return MathC.equals_float(this.x, p_other.x) &&
            MathC.equals_float(this.y, p_other.y) && MathC.equals_float(this.z, p_other.z);
    };
};

export class quat {
    public x: number;
    public y: number;
    public z: number;
    public w: number;

    public static build(p_x: number, p_y: number, p_z: number, p_w: number): quat {
        let l_return = new quat();
        l_return.x = p_x;
        l_return.y = p_y;
        l_return.z = p_z;
        l_return.w = p_w;
        return l_return;
    };

    public static rotate_around(p_axis: v3f, p_angle: number) {
        let l_sin = Math.sin(p_angle * 0.5);
        return quat.build(p_axis.x * l_sin, p_axis.y * l_sin, p_axis.z * l_sin, Math.cos(p_angle * 0.5));
    };

    public mul(p_number: number): quat {
        return quat.build(this.x * p_number, this.y * p_number, this.z * p_number, this.w * p_number);
    };

    public serialize(in_out_vector: Vector) {
        BinarySerializer.float32(in_out_vector, this.x);
        BinarySerializer.float32(in_out_vector, this.y);
        BinarySerializer.float32(in_out_vector, this.z);
        BinarySerializer.float32(in_out_vector, this.w);
    };

};

export class transform {
    public position: v3f;
    public rotation: quat;
    public scale: v3f;

    public static build(p_position: v3f, p_rotation: quat, p_scale: v3f): transform {
        let l_return = new transform();
        l_return.position = p_position;
        l_return.rotation = p_rotation;
        l_return.scale = p_scale;
        return l_return;
    };

    public serialize(in_out_vector: Vector) {
        this.position.serialize(in_out_vector);
        this.rotation.serialize(in_out_vector);
        this.scale.serialize(in_out_vector);
    };
};

export class CameraComponent {
    near: number;
    far: number;
    fov: number;

    public static build(p_near: number, p_far: number, p_fov: number): CameraComponent {
        let l_return: CameraComponent = new CameraComponent();
        l_return.near = p_near;
        l_return.far = p_far;
        l_return.fov = p_fov;
        return l_return;
    };

    public serialize(in_out_vector: Vector) {
        BinarySerializer.float32(in_out_vector, this.near);
        BinarySerializer.float32(in_out_vector, this.far);
        BinarySerializer.float32(in_out_vector, this.fov);
    };
};

export interface MeshRendererComponent {
};

export class Engine {
};

export class Node {
};

export class MeshRenderer {
};

export class EngineFunctions {

    public static SpawnEngine(p_database_path: string): Token<Engine> {
        let l_parameters: Vector = Vector.allocate(0);
        l_parameters.push_back(EngineFuncKey.allocate(EngineFunctionsKey.SpawnEngine).tok.to_slice());
        BinarySerializer.slice(l_parameters, Span.from_string(p_database_path).to_slice());
        let l_return = engine.EntryPoint(l_parameters.span.memory);

        return Token.allocate<Engine>(l_return);
    };

    public static DestroyEngine(p_engine: Token<Engine>) {
        let l_parameters: Vector = Vector.allocate(0);
        l_parameters.push_back(EngineFuncKey.allocate(EngineFunctionsKey.DestroyEngine).tok.to_slice());
        l_parameters.push_back(Slice.from_memory(p_engine.tok))
        engine.EntryPoint(l_parameters.span.memory);
    };

    public static MainLoopNonBlocking(p_engine: Token<Engine>): EngineLoopState {
        let l_parameters: Vector = Vector.allocate(0);
        l_parameters.push_back(EngineFuncKey.allocate(EngineFunctionsKey.MainLoopNonBlocking).tok.to_slice());
        l_parameters.push_back(Slice.from_memory(p_engine.tok))
        let l_return = engine.EntryPoint(l_parameters.span.memory);
        return BinaryDeserializer.int8(Slice.from_memory(l_return)) as EngineLoopState;
    };

    public static FrameBefore(p_engine: Token<Engine>) {
        let l_parameters: Vector = Vector.allocate(0);
        l_parameters.push_back(EngineFuncKey.allocate(EngineFunctionsKey.FrameBefore).tok.to_slice());
        l_parameters.push_back(Slice.from_memory(p_engine.tok))
        engine.EntryPoint(l_parameters.span.memory);
    };

    public static FrameAfter(p_engine: Token<Engine>) {
        let l_parameters: Vector = Vector.allocate(0);
        l_parameters.push_back(EngineFuncKey.allocate(EngineFunctionsKey.FrameAfter).tok.to_slice());
        l_parameters.push_back(Slice.from_memory(p_engine.tok))
        engine.EntryPoint(l_parameters.span.memory);
    };

    public static DeltaTime(p_engine: Token<Engine>): number {
        let l_parameters: Vector = Vector.allocate(0);
        l_parameters.push_back(EngineFuncKey.allocate(EngineFunctionsKey.DeltaTime).tok.to_slice());
        l_parameters.push_back(Slice.from_memory(p_engine.tok))
        let l_return = engine.EntryPoint(l_parameters.span.memory);
        return BinaryDeserializer.float32(Slice.from_memory(l_return));
    };

    public static FrameCount(p_engine: Token<Engine>): number {
        let l_parameters: Vector = Vector.allocate(0);
        l_parameters.push_back(EngineFuncKey.allocate(EngineFunctionsKey.FrameCount).tok.to_slice());
        l_parameters.push_back(Slice.from_memory(p_engine.tok))
        let l_return = engine.EntryPoint(l_parameters.span.memory);
        return BinaryDeserializer.int64(Slice.from_memory(l_return));
    };

    public static Node_CreateRoot(p_engine: Token<Engine>, p_transform: transform): Token<Node> {
        let l_parameters: Vector = Vector.allocate(0);
        l_parameters.push_back(EngineFuncKey.allocate(EngineFunctionsKey.Node_CreateRoot).tok.to_slice());
        l_parameters.push_back(Slice.from_memory(p_engine.tok))
        p_transform.serialize(l_parameters);
        let l_return = engine.EntryPoint(l_parameters.span.memory);
        return Token.allocate<Node>(l_return);
    };

    public static Node_Remove(p_engine: Token<Engine>, p_node: Token<Node>) {
        let l_parameters: Vector = Vector.allocate(0);
        l_parameters.push_back(EngineFuncKey.allocate(EngineFunctionsKey.Node_Remove).tok.to_slice());
        l_parameters.push_back(Slice.from_memory(p_engine.tok))
        l_parameters.push_back(Slice.from_memory(p_node.tok));
        engine.EntryPoint(l_parameters.span.memory);
    };

    public static Node_GetLocalPosition(p_engine: Token<Engine>, p_node: Token<Node>): v3f {
        let l_parameters: Vector = Vector.allocate(0);
        l_parameters.push_back(EngineFuncKey.allocate(EngineFunctionsKey.Node_GetLocalPosition).tok.to_slice());
        l_parameters.push_back(Slice.from_memory(p_engine.tok))
        l_parameters.push_back(Slice.from_memory(p_node.tok))
        let l_return = Slice.from_memory(engine.EntryPoint(l_parameters.span.memory));
        return v3f.deserialize(l_return);
    };

    public static Node_AddWorldRotation(p_engine: Token<Engine>, p_node: Token<Node>, p_delta_rotation: quat) {
        let l_parameters: Vector = Vector.allocate(0);
        l_parameters.push_back(EngineFuncKey.allocate(EngineFunctionsKey.Node_AddWorldRotation).tok.to_slice());
        l_parameters.push_back(Slice.from_memory(p_engine.tok))
        l_parameters.push_back(Slice.from_memory(p_node.tok))
        p_delta_rotation.serialize(l_parameters);
        Slice.from_memory(engine.EntryPoint(l_parameters.span.memory));
    };

    public static Node_AddCamera(p_engine: Token<Engine>, p_node: Token<Node>, p_camera: CameraComponent): Token<CameraComponent> {
        let l_parameters: Vector = Vector.allocate(0);
        l_parameters.push_back(EngineFuncKey.allocate(EngineFunctionsKey.Node_AddCamera).tok.to_slice());
        l_parameters.push_back(Slice.from_memory(p_engine.tok))
        l_parameters.push_back(Slice.from_memory(p_node.tok))
        p_camera.serialize(l_parameters);
        let l_return = engine.EntryPoint(l_parameters.span.memory);
        return Token.allocate<CameraComponent>(l_return);
    };

    public static Node_AddMeshRenderer(p_engine: Token<Engine>, p_node: Token<Node>, p_material: string, p_mesh: string): Token<MeshRenderer> {
        let l_parameters: Vector = Vector.allocate(0);
        l_parameters.push_back(EngineFuncKey.allocate(EngineFunctionsKey.Node_AddMeshRenderer).tok.to_slice());
        l_parameters.push_back(Slice.from_memory(p_engine.tok))
        l_parameters.push_back(Slice.from_memory(p_node.tok))
        BinarySerializer.slice(l_parameters, Span.from_string(p_material).to_slice());
        BinarySerializer.slice(l_parameters, Span.from_string(p_mesh).to_slice());
        let l_return = engine.EntryPoint(l_parameters.span.memory);
        return Token.allocate<MeshRenderer>(l_return);
    };
};


/*
console.log(process.pid);
let l_engine_token = EngineFunctions.SpawnEngine(path.join(__dirname, "../../../_asset/asset/MaterialViewer_Package/asset.db"));

let l_camera_node: Token<Node> = new Token<Node>();
let l_object_node: Token<Node> = new Token<Node>();
let l_main_loop = setInterval(() => {
    let l_loop_state: EngineLoopState = EngineFunctions.MainLoopNonBlocking(l_engine_token)
    switch (l_loop_state) {
        case EngineLoopState.FRAME:
            EngineFunctions.FrameBefore(l_engine_token);
        {
            let l_frame_count = EngineFunctions.FrameCount(l_engine_token);
            if (l_frame_count == 1) {
                l_camera_node = EngineFunctions.Node_CreateRoot(l_engine_token, transform.build(v3f.build(0, 0, 0), quat.build(0, 0, 0, 1), v3f.build(1, 1, 1)));
                EngineFunctions.Node_AddCamera(l_engine_token, l_camera_node, CameraComponent.build(1, 30, 45));
                l_object_node = EngineFunctions.Node_CreateRoot(l_engine_token, transform.build(v3f.build(0, 0, 5), quat.build(0, 0, 0, 1), v3f.build(1, 1, 1)));
                EngineFunctions.Node_AddMeshRenderer(l_engine_token, l_object_node, "material_2.json", "cube.obj");
            } else {

                let l_delta_time_s = EngineFunctions.DeltaTime(l_engine_token);
                EngineFunctions.Node_AddWorldRotation(l_engine_token, l_object_node, quat.rotate_around(v3f.build(0, 1, 0), 3 * l_delta_time_s));

            }

        }
            EngineFunctions.FrameAfter(l_engine_token);
            break;
        case EngineLoopState.ABORTED:
            EngineFunctions.Node_Remove(l_engine_token, l_camera_node);
            EngineFunctions.Node_Remove(l_engine_token, l_object_node);
            EngineFunctions.DestroyEngine(l_engine_token);
            engine.Finalize();
            clearInterval(l_main_loop);
            break;
    }
});
*/
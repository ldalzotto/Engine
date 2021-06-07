import {engine, EngineFunctions, EngineLoopState, transform, v3f, quat, CameraComponent} from "../src/engine"
import {assert_true, Token} from "../src/buffer_lib";
import * as path from "path"

function test() {

    let l_engine_token = EngineFunctions.SpawnEngine(path.join(__dirname, "../../../../_asset/asset/MaterialViewer_Package/asset.db"));
    let l_node: Token<Node> = new Token<Node>();

    let l_executed:boolean = false;
    while(!l_executed)
    {
        let l_loop_state: EngineLoopState = EngineFunctions.MainLoopNonBlocking(l_engine_token)
        switch (l_loop_state) {
            case EngineLoopState.FRAME:
                EngineFunctions.FrameBefore(l_engine_token);
            {
                let l_frame_count = EngineFunctions.FrameCount(l_engine_token);
                if (l_frame_count == 1) {
                    l_node = EngineFunctions.Node_CreateRoot(l_engine_token, transform.build(v3f.build(1, 2, 3), quat.build(0, 0, 0, 1), v3f.build(1, 1, 1)));
                    assert_true(EngineFunctions.Node_GetLocalPosition(l_engine_token, l_node).equals(v3f.build(1, 2, 3)));
                }
            }
                EngineFunctions.FrameAfter(l_engine_token);
                EngineFunctions.Node_Remove(l_engine_token, l_node);
                EngineFunctions.DestroyEngine(l_engine_token);
                engine.Finalize();
                l_executed = true;
                break;
        }
    }
};


test();
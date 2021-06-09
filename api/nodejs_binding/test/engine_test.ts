import {engine, Engine, EngineLoopState, transform, v3f, quat, CameraComponent} from "../src/engine"
import {assert_true, Token} from "../src/buffer_lib";
import * as path from "path"

function test() {

    let l_engine = Engine.spawn(path.join(__dirname, "../../../../_asset/asset/MaterialViewer_Package/asset.db"));
    let l_node: Token<Node> = new Token<Node>();

    let l_executed: boolean = false;
    while (!l_executed) {
        let l_loop_state: EngineLoopState = l_engine.mainloop_nonblocking();
        switch (l_loop_state) {
            case EngineLoopState.FRAME:
                l_engine.frame_before();
            {
                let l_frame_count = l_engine.framecount();
                if (l_frame_count == 1) {
                    l_node = l_engine.node_createroot(transform.build(v3f.build(1, 2, 3), quat.build(0, 0, 0, 1), v3f.build(1, 1, 1)));
                    assert_true(l_engine.node_get_localposition(l_node).equals(v3f.build(1, 2, 3)));
                }
            }
                l_engine.frame_after();
                l_engine.node_remove(l_node);
                l_engine.destroy();
                l_executed = true;
                break;
        }
    }
};

test();
engine.Finalize();
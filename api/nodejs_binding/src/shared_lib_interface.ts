type ArrayBufferOrFunction = ArrayBuffer | ((any: any) => any);

export interface IEngineSharedLibrary {
    Finalize(): void;

    LoadDynamicLib(p_path: string): void;

    EntryPoint(...p_args: ArrayBufferOrFunction[]): ArrayBuffer;
};

export function load_shared_library(p_path:string): IEngineSharedLibrary {
    let l_module = require("../../../../_install/build/Debug/NodeJsBinding") as IEngineSharedLibrary;
    l_module.LoadDynamicLib(p_path);
    return l_module;
};
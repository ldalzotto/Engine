import { BinaryDeserializer, BinarySerializer, ByteConst, MathC, Slice, Span, Token, Vector } from "./buffer_lib";
import { IEngineSharedLibrary, load_shared_library } from "./shared_lib_interface";

export let asset_compiler = load_shared_library("../../../../cmake-build-debug/api/AssetCompiler_API.dll");

enum AssetCompilerFunctions {
    DatabaseConnection_Allocate = 0,
    DatabaseConnection_Free = 1,
    AssetMetadataDatabase_Allocate = 10,
    AssetMetadataDatabase_Free = 11,
    AssetMetadataDatabase_GetPaths = 12
};


export class AssetCompilerFunctionsKey {
    tok: Span;

    static allocate(p_type: AssetCompilerFunctions): AssetCompilerFunctionsKey {
        let l_return = new AssetCompilerFunctionsKey();
        l_return.tok = Span.allocate(ByteConst.INT8);
        l_return.tok.to_slice().set_int8(p_type);
        return l_return;
    };
};

export class DatabaseConnection {
    token: Token<number>;

    public static allocate(p_path: string): DatabaseConnection {
        let l_return: DatabaseConnection = new DatabaseConnection();
        let l_parameters: Vector = Vector.allocate(0);
        l_parameters.push_back(AssetCompilerFunctionsKey.allocate(AssetCompilerFunctions.DatabaseConnection_Allocate).tok.to_slice());
        BinarySerializer.slice(l_parameters, Span.from_string(p_path).to_slice());
        l_return.token = Token.allocate<number>(asset_compiler.EntryPoint(l_parameters.span.memory));
        return l_return;
    };

    public free() {
        let l_parameters: Vector = Vector.allocate(0);
        l_parameters.push_back(AssetCompilerFunctionsKey.allocate(AssetCompilerFunctions.DatabaseConnection_Free).tok.to_slice());
        l_parameters.push_back(Slice.from_memory(this.token.tok));
        asset_compiler.EntryPoint(l_parameters.span.memory);
    };
};

export class AssetMetadataDatabase {
    token: Token<number>;

    public static allocate(p_database_connection: DatabaseConnection): AssetMetadataDatabase {
        let l_return: AssetMetadataDatabase = new AssetMetadataDatabase();
        let l_parameters: Vector = Vector.allocate(0);
        l_parameters.push_back(AssetCompilerFunctionsKey.allocate(AssetCompilerFunctions.AssetMetadataDatabase_Allocate).tok.to_slice());
        l_parameters.push_back(Slice.from_memory(p_database_connection.token.tok));
        l_return.token = Token.allocate<number>(asset_compiler.EntryPoint(l_parameters.span.memory));
        return l_return;
    };

    public free(p_database_connection: DatabaseConnection) {
        let l_parameters: Vector = Vector.allocate(0);
        l_parameters.push_back(AssetCompilerFunctionsKey.allocate(AssetCompilerFunctions.AssetMetadataDatabase_Free).tok.to_slice());
        l_parameters.push_back(Slice.from_memory(p_database_connection.token.tok));
        l_parameters.push_back(Slice.from_memory(this.token.tok));
        asset_compiler.EntryPoint(l_parameters.span.memory);
    };


    public get_asset_paths(p_database_connection: DatabaseConnection, p_asset: string): Array<string> {
        let l_parameters: Vector = Vector.allocate(0);
        l_parameters.push_back(AssetCompilerFunctionsKey.allocate(AssetCompilerFunctions.AssetMetadataDatabase_GetPaths).tok.to_slice());
        l_parameters.push_back(Slice.from_memory(p_database_connection.token.tok));
        l_parameters.push_back(Slice.from_memory(this.token.tok));
        BinarySerializer.slice(l_parameters, Span.from_string(p_asset).to_slice());
        let l_return = asset_compiler.EntryPoint(l_parameters.span.memory);
        let l_paths = BinaryDeserializer.varyingslice(Slice.from_memory(l_return)).to_array_of_array();
        return l_paths.map((p_slice: Slice) => BinaryDeserializer.string(p_slice));
    }

};


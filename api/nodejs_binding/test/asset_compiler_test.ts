
import {DatabaseConnection, AssetMetadataDatabase} from "../src/asset_compiler"
import * as path from "path";

console.log(process.pid);

let l_db_connection = DatabaseConnection.allocate(path.join(__dirname, "../../../../_asset/asset/MaterialViewer_Package/asset.db"));
let l_asset_metadata = AssetMetadataDatabase.allocate(l_db_connection);
let l_material_assets = l_asset_metadata.get_asset_paths(l_db_connection, "MATERIAL");
l_asset_metadata.free(l_db_connection);
l_db_connection.free();

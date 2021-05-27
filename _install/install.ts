import * as child_process from 'child_process';
import * as https from 'https';
import * as fs from 'fs';
import * as path from 'path';

export enum ConfigurationType {
    UNDEFINED = 0,
    DEBUG = 1,
    RELEASE = 2
    // 4
};


namespace File {
    export function create_dir_if_not_exists(p_path: string) {
        if (!fs.existsSync(p_path)) {
            fs.mkdirSync(p_path);
        }
    };
    export function delete_dir_if_exists(p_path: string) {
        if (fs.existsSync(p_path)) {
            fs.rmdirSync(p_path, { recursive: true });
        }
    };
};

class HttpGetReturn {
    success: boolean;
    value: string;
};

function http_get(p_url: string): Promise<HttpGetReturn> {
    return new Promise<HttpGetReturn>((resolve, reject) => {
        console.log(`http get : ${p_url}`);
        let l_return: HttpGetReturn = new HttpGetReturn();
        https.get(p_url, (res) => {
            let l_body = "";
            res.on("data", (p_body) => {
                l_body += p_body.toString();
            });
            res.on("end", () => {
                l_return.success = true;
                l_return.value = l_body;
                resolve(l_return);
            }).on("error", (err) => {
                console.error(err);
                l_return.success = false;
                reject(l_return);
            });
        }).on("error", (err) => {
            console.error(err);
            l_return.success = false;
            reject(l_return);
        });
    });
};

function http_get_file_sync(p_url: string, p_path: string): Promise<boolean> {
    return new Promise<boolean>((resolve, reject) => {
        console.log(`http_file get : ${p_url}, to ${p_path}`);

        let l_return: boolean = false;

        const file = fs.createWriteStream(p_path);
        https.get(p_url, (res) => {
            res.pipe(file);
            file.on("finish", () => {
                file.close();
                l_return = true;
                resolve(l_return);
            })
        }).on("error", (err) => {
            console.error(err);
            l_return = false;
            reject(l_return);
        });
    });
};

function environment_variable_assertion(p_name: string): boolean {
    console.log(`Checking prensence of environment variable : ${p_name}`);
    if (process.env[p_name] == undefined) {
        console.error(`ERROR : The environment variable ${p_name} must be defined !`);
        return false;
    }
    return true;
};

namespace ChildProcess {
    export class Commands {
        public static execute(p_command: string): boolean {
            console.log(`ChildProcess : execute command ${p_command}`);
            try {
                child_process.execSync(p_command, { stdio: "inherit" });
                return true;
            } catch (error) {
                console.error(error);
                return false;
            }
        };

        public static move(p_new_dir: string): boolean {
            try {
                console.log(`ChildProcess : moving from ${process.cwd()} to ${p_new_dir}`);
                process.chdir(p_new_dir);
                return true;
            } catch (error) {
                console.error(error);
                return false;
            }
        };
    }
};

namespace cmake {
    export class Commands {
        public static config_and_install(p_command_folder: string, p_cmake_config_type: ConfigurationType) {
            let l_install_folder_relative: string = `./install/${ConfigurationType[p_cmake_config_type].toLowerCase()}`;
            let l_configuration_type: string = "";
            switch (p_cmake_config_type) {
                case ConfigurationType.DEBUG:
                    l_configuration_type = "Debug";
                    break;
                case ConfigurationType.RELEASE:
                    l_configuration_type = "Release";
                    break;
            }


            ChildProcess.Commands.move(p_command_folder)
            ChildProcess.Commands.execute(`cmake -DCMAKE_INSTALL_PREFIX=\"${l_install_folder_relative}\" .`);
            ChildProcess.Commands.execute(`cmake --build . --config ${l_configuration_type} --target install`);
            ChildProcess.Commands.move(__dirname);
            return l_install_folder_relative;
        };
    };
};

namespace szip {
    export class Commands {
        public static unzip_to(p_zip_file_path: string, p_extract_path: string): boolean {
            console.log(`szip : unzip ${p_zip_file_path} to ${p_extract_path}`);
            return ChildProcess.Commands.execute(`7z x ${p_zip_file_path} -o${p_extract_path}`);
        };
    };
};

namespace anonfile {
    export class Commands {
        public static async download_to(p_file_id: string, p_path: string): Promise<boolean> {
            console.log(`anonfile : retrieve url of ${p_file_id}`);
            let l_get_website: HttpGetReturn = await http_get(`https://anonfiles.com/${p_file_id}`);
            if (l_get_website.success) {
                let l_url_start_index = l_get_website.value.indexOf('id="download-url"');
                if (l_url_start_index !== -1) {
                    let l_url_begin_index = l_get_website.value.indexOf("https://", l_url_start_index);
                    let l_url_end_index = l_get_website.value.indexOf('"', l_url_begin_index);
                    let l_url = l_get_website.value.slice(l_url_begin_index, l_url_end_index);
                    console.log(`anonfile : url found ${l_url}`);
                    return await http_get_file_sync(l_url, p_path);
                }
            }

            console.log(`anonfile : error for file ${p_file_id}`);
            return false;
        };

        public static async download_and_extract(p_file_id: string, p_tmp_file_path: string, p_extract_path: string): Promise<boolean> {
            if (!await Commands.download_to(p_file_id, p_tmp_file_path)) {
                console.error("ERROR : sqllite3_prebuild_binaries");
                return false;
            };

            return szip.Commands.unzip_to(p_tmp_file_path, p_extract_path);
        };
    };
};

class Folders {
    root: string;
    tmp_folder_absolute: string;
    tmp_file_name: string;
    third_party_folder_absolute: string;

    public initialize() {
        File.create_dir_if_not_exists(path.join(this.root, this.tmp_folder_absolute));
        File.create_dir_if_not_exists(this.get_third_party_file_path());
    };

    public clear() {
        fs.rmSync(path.join(this.root, this.tmp_folder_absolute), { recursive: true });
    };

    public get_temp_file_path(): string {
        return path.join(this.root, this.tmp_folder_absolute, this.tmp_file_name);
    };

    public get_temp_folder_path(): string {
        return path.join(this.root, this.tmp_folder_absolute);
    };

    public get_third_party_file_path(): string {
        return path.join(this.root, this.third_party_folder_absolute);
    };
};

enum ThirdPartyType {
    SQLITE3,
    VULKAN,
    IMGUI,
    STB_IMAGE,
    GLSLANG
};

class Constants {
    public static vulkan_windows_file_id = "n8p8ieyau0/vulkan_windows_27_05_2021_7z";
    public static vulkan_linux_file_id = "z399idycu1/vulkan_linux_07_05_2021_7z";

    public static imgui_file_id = "Z3D1iay4u1/imgui_27_05_2021_7z";
    public static stb_image_file_id = "l0r6b3y8u6/stb_image_26_05_2021_7z";
    public static sqlite3_source_id = "z7s7c5y1uc/sqlite_source_26_05_2021_7z";
    public static glslang_source_id = "l6E2c2y0ue/glslang_source_26_05_2021_7z";
};

async function third_party_prebuilt(p_third_party: ThirdPartyType, p_folders: Folders): Promise<boolean> {
    let l_file_id: string = "";
    switch (process.platform) {
        case "win32":
            switch (p_third_party) {
                case ThirdPartyType.VULKAN:
                    l_file_id = Constants.vulkan_windows_file_id;
                    break;
                case ThirdPartyType.STB_IMAGE:
                    l_file_id = Constants.stb_image_file_id;
                    break;
                case ThirdPartyType.IMGUI:
                    l_file_id = Constants.imgui_file_id;
                    break;
                default:
                    console.log(`Third party ${p_third_party} not supported.`);
                    return false;
            }
            break;
        case "linux":
            switch (p_third_party) {
                case ThirdPartyType.VULKAN:
                    l_file_id = Constants.vulkan_linux_file_id;
                    break;
                case ThirdPartyType.STB_IMAGE:
                    l_file_id = Constants.stb_image_file_id;
                    break;
                case ThirdPartyType.IMGUI:
                    l_file_id = Constants.imgui_file_id;
                    break;
                default:
                    console.log(`Third party ${p_third_party} not supported.`);
                    return false;
            }
            break;
        default:
            console.error(`Platform ${process.platform} not supported.`);
            return false;
    }

    return await anonfile.Commands.download_and_extract(l_file_id, p_folders.get_temp_file_path(), p_folders.get_third_party_file_path());
};

async function third_party_compile(p_third_party: ThirdPartyType, p_cmake_config_types: Array<ConfigurationType>, p_folders: Folders): Promise<boolean> {

    let l_file_id: string = "";
    let l_source_tmp_relative_folder = "";
    let l_third_party_relative_folder = "";

    switch (p_third_party) {
        case ThirdPartyType.SQLITE3:
            l_file_id = Constants.sqlite3_source_id;
            l_source_tmp_relative_folder = "sqlite_source";
            l_third_party_relative_folder = "sqlite";
            break;
        case ThirdPartyType.GLSLANG:
            l_file_id = Constants.glslang_source_id;
            l_source_tmp_relative_folder = "glslang_source";
            l_third_party_relative_folder = "glslang";
            break;
        default:
            console.log(`Third party ${p_third_party} not supported.`);
            return false;
    }

    File.create_dir_if_not_exists(path.join(p_folders.get_third_party_file_path(), l_third_party_relative_folder));

    if (!await anonfile.Commands.download_and_extract(l_file_id, p_folders.get_temp_file_path(), p_folders.get_temp_folder_path())) {
        return false;
    };

    let l_sqlite_source_root_folder: string = path.join(p_folders.get_temp_folder_path(), l_source_tmp_relative_folder);

    for (let i = 0; i < p_cmake_config_types.length; i++) {
        let l_install_folder_relative = cmake.Commands.config_and_install(l_sqlite_source_root_folder, p_cmake_config_types[i]);
        let l_copy_dir = path.join(p_folders.get_third_party_file_path(), l_third_party_relative_folder, ConfigurationType[p_cmake_config_types[i]].toString().toLowerCase());
        File.delete_dir_if_exists(l_copy_dir);
        fs.renameSync(path.join(l_sqlite_source_root_folder, l_install_folder_relative), l_copy_dir);
    }

    return true;
};

async function third_party(p_folders: Folders): Promise<boolean> {
    if (!await third_party_compile(ThirdPartyType.SQLITE3, [ConfigurationType.RELEASE], p_folders)) {
        return false;
    }
    if (!await third_party_compile(ThirdPartyType.GLSLANG, [ConfigurationType.DEBUG, ConfigurationType.RELEASE], p_folders)) {
        return false;
    }

    if (!environment_variable_assertion("VULKAN_SDK")) {
        console.error("The Vulkan SDK installation cannot be detected. Please, install it from here https://vulkan.lunarg.com/ and make sure that the environment variable named \"VULKAN_SDK\" points to the installation root folder.");
        return false;
    }

    if (process.platform === "linux") {
        if (!environment_variable_assertion("LD_LIBRARY_PATH")) {
            console.error("The Vulkan SDK installation is not complete. Please, set the environment variable named \"LD_LIBRARY_PATH\" according to https://vulkan.lunarg.com/doc/view/1.1.126.0/linux/getting_started.html.");
            return false;
        }
        if (!environment_variable_assertion("VK_LAYER_PATH")) {
            console.error("The Vulkan SDK installation is not complete. Please, set the environment variable named \"VK_LAYER_PATH\" according to https://vulkan.lunarg.com/doc/view/1.1.126.0/linux/getting_started.html.");
            return false;
        }
    }

    if (!await third_party_prebuilt(ThirdPartyType.VULKAN, p_folders)) {
        return false;
    }
    if (!await third_party_prebuilt(ThirdPartyType.STB_IMAGE, p_folders)) {
        return false;
    }
    if (!await third_party_prebuilt(ThirdPartyType.IMGUI, p_folders)) {
        return false;
    }

    return true;
};

async function main() {

    let l_install_folder_relative = process.argv[2];

    let l_folders: Folders = new Folders();
    l_folders.root = path.join(__dirname, "../", l_install_folder_relative);
    l_folders.tmp_folder_absolute = ".tmp";
    l_folders.tmp_file_name = "file";
    l_folders.third_party_folder_absolute = "ThirdParty";
    l_folders.initialize();

    await third_party(l_folders);

    l_folders.clear();

};

process.on('unhandledRejection', (reason, p) => {
    console.error('Unhandled Rejection at:', p, 'reason:', reason)
    process.exit(1)
});

main();
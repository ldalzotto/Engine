
import * as path from 'path';
import * as child_process from 'child_process';
import * as fs from 'fs';


enum BuildType
{
    DEBUG = <any>"DEBUG",
    RELEASE_DEBUG = <any>"RELEASE_DEBUG",
    RELEASE = <any>"RELEASE"
};

enum CompilerType
{
    GCC = <any>"GCC",
    MSVC = <any>"MSVC"
};

class CompileTypeProvider
{
    public static compiler(p_compiler_type: CompilerType): string
    {
        if (p_compiler_type == CompilerType.GCC)
        {
            return "gcc";
        }
        else if (p_compiler_type == CompilerType.MSVC)
        {
            return "cl";
        }
    };

    public static math_link(p_compiler_type: CompilerType): string
    {
        if (p_compiler_type == CompilerType.GCC)
        {
            return "-lm";
        }
        return "";
    };

    public static wall(p_compiler_type: CompilerType): string
    {
        if (p_compiler_type == CompilerType.GCC)
        {
            return "-Wall";
        }
        else if (p_compiler_type == CompilerType.MSVC)
        {
            return "";
        }
    };

    public static preprocessor(p_compiler_type: CompilerType, p_preprocessor_name: string, p_preprocessor_value: string): string
    {
        if (p_compiler_type == CompilerType.GCC)
        {
            return `-D ${p_preprocessor_name}=${p_preprocessor_value}`;
        }
        else if (p_compiler_type == CompilerType.MSVC)
        {
            return `/D"${p_preprocessor_name}=${p_preprocessor_value}"`;
        }
    };

    public static debug_symbols(p_compiler_type: CompilerType): string
    {
        if (p_compiler_type == CompilerType.GCC)
        {
            return "-g";
        }
        else if (p_compiler_type == CompilerType.MSVC)
        {
            return "/Zi";
        }
    };

    public static optimization(p_compiler_type: CompilerType, p_build_type: BuildType): string
    {
        let l_optimisation_flags = "";
        switch (p_build_type)
        {
            case BuildType.DEBUG:
                if (p_compiler_type == CompilerType.GCC)
                {
                    l_optimisation_flags += "-O0";
                }
                break;
            case BuildType.RELEASE_DEBUG:
                if (p_compiler_type == CompilerType.GCC)
                {
                    l_optimisation_flags += "-O2";
                }
                else if (p_compiler_type == CompilerType.MSVC)
                {
                    l_optimisation_flags += "/O2";
                }
                break;
            case BuildType.RELEASE:
                if (p_compiler_type == CompilerType.GCC)
                {
                    l_optimisation_flags += "-O2";
                }
                else if (p_compiler_type == CompilerType.MSVC)
                {
                    l_optimisation_flags += "/O2";
                }
                break;
        };
        return l_optimisation_flags;
    };

    public static include_dir(p_compiler_type: CompilerType, p_dir: string): string
    {
        if (p_compiler_type == CompilerType.GCC)
        {
            return `-I${p_dir}`;
        }
        else if (p_compiler_type == CompilerType.MSVC)
        {
            return `/I${p_dir}`;
        }
    }
};

class PreprocessorConstants2
{
    public DEBUG: string;
    public RELEASE_DEBUG: string;
    public RELEASE: string;
};

class RootPath
{
    public static evaluate(): string
    {
        let l_root_path_dir: string[] = __dirname.split(path.sep);
        return l_root_path_dir.splice(0, l_root_path_dir.length - 1).join(path.sep);
    };
}

const root_path: string = RootPath.evaluate();
const build_directory: string = path.join(root_path, "build");

class Command
{
    public command: string;
    public file: string;

    public constructor(p_command: string, p_file: string)
    {
        this.command = p_command;
        this.file = p_file;
    }

    public execute()
    {
        console.log(this.command);
        child_process.execSync(this.command);
    };
};


class CommandConfiguration
{
    public name: string;
    public build_type?: BuildType;
    public global_link_flags: string;
    public project_name: string;
    public project_root: string;
    public main_file: string;
    public include_directories: string[];

    public build_command(p_compiler_type: CompilerType, p_preprocessor_constants: PreprocessorConstants2): Command
    {

        let l_debug_flags = CompileTypeProvider.wall(p_compiler_type);
        if (this.build_type == BuildType.DEBUG
            || this.build_type == BuildType.RELEASE_DEBUG)
        {
            l_debug_flags += ` ${CompileTypeProvider.debug_symbols(p_compiler_type)}`;
        };

        let l_optimisation_flags = CompileTypeProvider.optimization(p_compiler_type, this.build_type);

        //preprocessors
        let l_preprocessor_values = p_preprocessor_constants[this.build_type];

        const c: string = CompileTypeProvider.compiler(p_compiler_type);
        let l_command: string = `${c} ${l_debug_flags} ${l_optimisation_flags} ${l_preprocessor_values} ${this.main_file}`;
        this.include_directories.forEach(_include_dir => { l_command += ` ${CompileTypeProvider.include_dir(p_compiler_type, _include_dir)}` });
        l_command += ` -o ${path.join(build_directory, this.project_name) + '_' + this.build_type + '.exe'} ${this.global_link_flags} `;
        return new Command(l_command, this.main_file);
    };
};


enum ModuleType
{
    INTERFACE = <any>"INTERFACE",
    EXECUTABLE = <any>"EXECUTABLE",
    STATIC_LIBRARY = <any>"STATIC_LIBRARY",
    DYNAMIC_LIBRARY = <any>"DYNAMIC_LIBRARY"
};


class EnvironmentEntry
{
    public name: string;
    public value: string;
};

class Environment
{
    public DEBUG: EnvironmentEntry[];
    public RELEASE_DEBUG: EnvironmentEntry[];
    public RELEASE: EnvironmentEntry[];
};


class BuildConfigurationDependency
{
    public module: string;
};

class BuildConfigurationEntry
{
    public base_name: string;
    public module_type: ModuleType;
    public build_type: BuildType;
    public root_folder: string;
    public main_file?: string; //optional for interfaces
    public include_directories: string[];
    public dependencies: BuildConfigurationDependency[]; //strings are base_names

    public static build_command_configuration(p_entry: BuildConfigurationEntry, p_compiler_type: CompilerType): CommandConfiguration
    {
        let command_config: CommandConfiguration = new CommandConfiguration();
        command_config.name = p_entry.base_name;
        command_config.build_type = p_entry.build_type;
        command_config.global_link_flags = CompileTypeProvider.math_link(p_compiler_type); //for math
        command_config.project_name = p_entry.base_name;
        command_config.project_root = path.join(root_path, p_entry.root_folder);
        command_config.main_file = path.join(command_config.project_root, p_entry.main_file);
        command_config.include_directories = p_entry.include_directories.map(_include => path.join(root_path, _include));
        return command_config;
    };
};

class BuildConfiguration
{
    public compiler: CompilerType;
    public environment: Environment;
    public modules: BuildConfigurationEntry[];
};



// ################ loading configuration

const build_configuration: BuildConfiguration = JSON.parse(fs.readFileSync(path.join(root_path, "build_config.json")).toString());

// ################ ENVIRONMENT


class PreprocessorBuilder
{
    public static concat_environment_entries(p_compiler_type: CompilerType, p_entries: EnvironmentEntry[]): string
    {
        let l_return: string = "";
        p_entries.forEach((_entry: EnvironmentEntry) =>
        {
            l_return += " ";
            l_return += CompileTypeProvider.preprocessor(p_compiler_type, _entry.name, _entry.value);
            l_return += " ";
        });
        return l_return;
    };

    public static build(p_compiler_type: CompilerType, p_environment: Environment): PreprocessorConstants2
    {
        return {
            DEBUG: this.concat_environment_entries(p_compiler_type, p_environment.DEBUG),
            RELEASE_DEBUG: this.concat_environment_entries(p_compiler_type, p_environment.RELEASE_DEBUG),
            RELEASE: this.concat_environment_entries(p_compiler_type, p_environment.RELEASE)
        };
    };
};

let preprocessor_constants: PreprocessorConstants2 = PreprocessorBuilder.build(build_configuration.compiler, build_configuration.environment);


// #############  MODULES

interface BuildModules
{
    [key: string]: BuildConfigurationEntry;
};

let build_modules: BuildModules = {};

build_configuration.modules.forEach((p_entry: BuildConfigurationEntry) =>
{
    build_modules[p_entry.base_name] = p_entry;
});

class InterfaceModule
{
    public include_directories: string[];

    public constructor() { this.include_directories = []; }
};

interface InterfaceModules
{
    [key: string]: InterfaceModule;
};

class InterfaceModuleBuilder
{
    public static build_recursively(p_compiler_type: CompilerType, p_module: BuildConfigurationEntry, p_modules: BuildModules,
        in_out_interfacemodule: InterfaceModule)
    {
        p_module.dependencies.forEach((_parent: BuildConfigurationDependency) =>
        {
            this.build_recursively(p_compiler_type, p_modules[_parent.module], p_modules, in_out_interfacemodule);
        });

        in_out_interfacemodule.include_directories =
            in_out_interfacemodule.include_directories.concat(p_module.include_directories);
    };
};


let interface_modules: InterfaceModules = {};

let l_all_command_configurations: CommandConfiguration[] = [];

Object.keys(build_modules).forEach((p_module_key: string) =>
{
    let l_module: BuildConfigurationEntry = build_modules[p_module_key];
    if (l_module.module_type == ModuleType.INTERFACE)
    {
        let l_interface_module: InterfaceModule = new InterfaceModule();
        InterfaceModuleBuilder.build_recursively(build_configuration.compiler, l_module, build_modules, l_interface_module);
        interface_modules[p_module_key] = l_interface_module;
    }
    else if (l_module.module_type == ModuleType.EXECUTABLE)
    {
        let l_executable_command: CommandConfiguration =
            BuildConfigurationEntry.build_command_configuration(l_module, build_configuration.compiler);

        l_module.dependencies.forEach((_dependency: BuildConfigurationDependency) =>
        {
            if (interface_modules[_dependency.module])
            {
                l_executable_command.include_directories =
                    l_executable_command.include_directories.concat(interface_modules[_dependency.module].include_directories);
            }
        });

        l_all_command_configurations.push(l_executable_command);
    }
});


let l_all_commands: Command[] = l_all_command_configurations.map(_command_conf =>
    _command_conf.build_command(build_configuration.compiler, preprocessor_constants));

l_all_commands.forEach(_command => _command.execute());

// compile_json_gneeration
class CommandGenerationEntry
{
    public directory: string;
    public command: string;
    public file: string;

    public constructor(p_directory: string, p_command: string, p_file: string)
    {
        this.directory = p_directory;
        this.command = p_command;
        this.file = p_file;
    };
}

let l_compile_json_generation_file: string = path.join(build_directory, "compile_commands.json");
let compile_json_generation = JSON.stringify(
    l_all_commands.map(_command => new CommandGenerationEntry(build_directory, _command.command, _command.file))
);

fs.writeFileSync(l_compile_json_generation_file, compile_json_generation);
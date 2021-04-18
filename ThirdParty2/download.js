const https = require('https');
const fs = require('fs');
const path = require('path');
const child_process = require('child_process');

const http_get = function(p_url, p_on_success, p_on_error) {
    https.get(p_url, (res) => {
        let l_body = "";
        res.on("data", (p_body) => {
            l_body += p_body.toString();
        });
        res.on("end", () => {
            p_on_success(l_body);
        }).on("error", (err) => {
            console.error(err);
            if (p_on_error) {
                p_on_error(err);
            }
        });
    });
};

const http_get_file = function(p_url, p_path, p_on_success, p_on_error) {
    const file = fs.createWriteStream(p_path);
    https.get(p_url, (res) => {
        res.pipe(file);
        file.on("finish", () => {
            file.close();
            if (p_on_success) {
                p_on_success();
            }
        })
    }).on("error", (err) => {
        console.error(err);
        if (p_on_error) {
            p_on_error(err);
        }
    });
};

const get_file_from_filehost = function(p_file_id, p_path, p_on_success, p_on_error) {
    http_get(`https://anonfiles.com/${p_file_id}`, (p_res) => {
        let l_url_start_index = p_res.indexOf('id="download-url"');
        if (l_url_start_index !== -1) {
            let l_url_begin_index = p_res.indexOf("https://", l_url_start_index);
            let l_url_end_index = p_res.indexOf('"', l_url_begin_index);
            let l_url = p_res.slice(l_url_begin_index, l_url_end_index);
            http_get_file(l_url, p_path, p_on_success, p_on_error);
        } else {
            if (p_on_error) {
                p_on_error();
            }
        }
    }, p_on_error);
};

const unzip_to = function(p_zip_file_path, p_extract_path, p_on_success, p_on_error) {
    fs.rmdir(p_extract_path, { recursive: true }, (err) => {
        if (err) {
            if (p_on_error) {
                p_on_error(err);
            }
            return;
        }
        fs.mkdir(p_extract_path, { recursive: true }, (err) => {
            if (err) {
                if (p_on_error) {
                    p_on_error(err);
                    return;
                }
            }

            child_process.exec(`7z x ${p_zip_file_path} -o${p_extract_path}`, {}, (error, stdout, stderr) => {
                if (error) {
                    if (p_on_error) {
                        p_on_error(err);
                    }
                    return;
                }

                if (p_on_success) {
                    p_on_success();
                }

            });
        });
    });
};

const git_checkout = function(p_repo_root_path, p_commit_hash, p_on_success, p_on_error) {
    let l_old_folder = __dirname;
    process.chdir(p_repo_root_path);
    child_process.exec(`git checkout ${p_commit_hash}`, {}, (error, stdout, stderr) => {
        process.chdir(l_old_folder);
        if (error) {
            if (p_on_error) {
                p_on_error(error);
            }
        }

        if (p_on_success) {
            p_on_success();
        }
    });
};

const zip_temp_folder_path = path.join(__dirname, "./.tmp");
const zip_temp_file_path = path.join(zip_temp_folder_path, "./zip.7z");

const close_app = function() {
    fs.rmdirSync(zip_temp_folder_path, { recursive: true });
};

fs.mkdir(zip_temp_folder_path, { recursive: true }, (err) => {
    if (err) {
        console.error(err);
        close_app();
        return;
    }

    fs.readFile(path.join(__dirname, "./zip_download.json"), (err, data) => {
        if (err) {
            console.error(err);
            close_app();
            return;
        }

        let l_download_configuration = JSON.parse(data);

        let l_index = 0;

        const foreach_func = function() {
            if (l_index < l_download_configuration.length) {
                let l_download_instance = l_download_configuration[l_index];

                if (l_download_instance.relative_extract_path.indexOf("../") !== -1) {
                    console.error("A path contains, '../', this is forbidden.");
                    close_app();
                    return;
                }
                if (__dirname === path.join(__dirname, l_download_instance.relative_extract_path)) {
                    console.error("Cannot extract to root");
                    close_app();
                    return;
                }

                http_get("https://gameengine-74abb-default-rtdb.europe-west1.firebasedatabase.app/.json", (res) => {
                    let l_db_index = JSON.parse(res);

                    get_file_from_filehost(l_db_index[l_download_instance.file_id], zip_temp_file_path, () => {
                            let l_target_folder_path = path.join(__dirname, l_download_instance.relative_extract_path);
                            unzip_to(zip_temp_file_path, l_target_folder_path, () => {
                                if (l_download_instance.git) {
                                    git_checkout(l_target_folder_path, l_download_instance.git.commit_hash, () => {
                                        l_index += 1;
                                        foreach_func();
                                    }, (error) => {
                                        if (error) {
                                            console.error(error);
                                            close_app();
                                            return;
                                        }
                                    });
                                } else {
                                    l_index += 1;
                                    foreach_func();
                                }
                            }, (error) => {
                                if (error) {
                                    console.error(error);
                                    close_app();
                                    return;
                                }
                            });
                        },
                        () => {
                            close_app();
                        });

                    /* 
                    http_get_file(`https://cdn-32.anonfiles.com/${l_db_index[l_download_instance.file_id]}`, zip_temp_file_path, () => {
                            let l_target_folder_path = path.join(__dirname, l_download_instance.relative_extract_path);
                            unzip_to(zip_temp_file_path, l_target_folder_path, () => {
                                if (l_download_instance.git) {
                                    git_checkout(l_target_folder_path, l_download_instance.git.commit_hash, () => {
                                        l_index += 1;
                                        foreach_func();
                                    }, (error) => {
                                        if (error) {
                                            console.error(error);
                                            close_app();
                                            return;
                                        }
                                    });
                                } else {
                                    l_index += 1;
                                    foreach_func();
                                }
                            }, (error) => {
                                if (error) {
                                    console.error(error);
                                    close_app();
                                    return;
                                }
                            });
                        },
                        () => {
                            close_app();
                        });
                    */
                }, () => {
                    console.error("Cannot connect to db");
                    close_app();
                    return;
                })



            } else {
                close_app();
            }
        };

        foreach_func();
    });
})
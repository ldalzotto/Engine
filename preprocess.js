const fs = require('fs');
const child_process = require("child_process");
var args = process.argv.slice(2);
const l_source_path = args[0];
const l_target_path = args[1];
child_process.execSync(`clang -D__PREPROCESS=1 -D__DEBUG=1 -D__RELASE=0 -E -P -C -CC ${l_source_path} > ${l_target_path}`);
child_process.execSync(`clang-format -i -style=WebKit ${l_target_path}`);

const l_target_content = fs.readFileSync(l_target_path).toString();
const l_splitted_content = l_target_content.split('\n');
for (let i = l_splitted_content.length - 1; i >= 0; i--) {
    if (l_splitted_content[i].startsWith("#")) {
        l_splitted_content.splice(i, 1);
    } else if (l_splitted_content[i].startsWith("@")) {
        l_splitted_content[i] = l_splitted_content[i].replace("@", "#");
    }
}
fs.writeFileSync(l_target_path, l_splitted_content.join('\n'));


// child_process.execSync(`clang-format -style=file "${l_target_path}"`);
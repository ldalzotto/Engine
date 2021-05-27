const child_process = require("child_process");
const fs = require("fs");
const path = require("path");

try {
    child_process.execSync("tsc -v");
} catch (error) {
    console.error("Typescript is not installed globally.");
    process.exit(1);
}

try {
    child_process.execSync("node -v");
} catch (error) {
    console.error("Node is not installed.");
    process.exit(1);
}

try {
    child_process.execSync("tsc ./install.ts --outDir ./.gen --sourcemap", { stdio: 'inherit' });
    child_process.execSync("node ./.gen/install.js ../", { stdio: 'inherit' });
    fs.rmdirSync(path.join(__dirname, "./.gen"), { recursive: true });
} catch (error) {
    console.error(error);
    process.exit(1);
}
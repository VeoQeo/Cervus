#!/usr/bin/env node

import { execSync } from 'child_process';
import fs from 'fs/promises';
import path from 'path';

const ASM_EXT = /\.asm$/i;

async function clearBuildArtifacts(directory, baseName) {
    const files = await fs.readdir(directory);
    const toRemove = files.filter(f =>
        f === `${baseName}.o` ||
        f === baseName ||                      // Linux executable
        f === `${baseName}.exe` ||             // Windows
        (f.startsWith(baseName) && f.endsWith('.out')) 
    );

    if (toRemove.length === 0) return;

    console.log(`\  deleting old artefact: ${toRemove.join(', ')}`);
    for (const file of toRemove) {
        await fs.unlink(path.join(directory, file));
    }
}

function parseArgs(rawArgs) {
    const args = { source: null, output: null, link: false, linkMode: 'standalone' };
    let i = 0;

    while (i < rawArgs.length) {
        const arg = rawArgs[i];

        if (arg === '-l' || arg === '--link') {
            args.link = true;
            if (i + 1 < rawArgs.length && !rawArgs[i + 1].startsWith('-')) {
                const next = rawArgs[i + 1].toLowerCase();
                if (next === 'gcc') {
                    args.linkMode = 'gcc';
                    i++; 
                }
            }
        } else if (!args.source) {
            args.source = arg;
        } else if (!args.output) {
            args.output = arg;
        } else {
            console.error(`unknow arg: ${arg}`);
            process.exit(1);
        }
        i++;
    }

    if (!args.source) {
        console.error('error: dont point file');
        process.exit(1);
    }

    return args;
}

async function main() {
    const rawArgs = process.argv.slice(2);
    if (rawArgs.length === 0) {
        console.log('using');
        console.log('  node make.js <file.asm> [output]');
        console.log('  node make.js <file> -l                # link with ld с _start');
        console.log('  node make.js <file> -l gcc            # link from gcc (для main + libc)');
        console.log('Пример:');
        console.log('  node make.js hello -l');
        process.exit(1);
    }

    const opts = parseArgs(rawArgs);
    const dir = process.cwd();

    let baseName = opts.source.replace(ASM_EXT, '');
    const outName = opts.output || baseName;

    const files = await fs.readdir(dir);
    const asmFile = files.find(f =>
        f.toLowerCase() === `${baseName}.asm`.toLowerCase()
    );

    if (!asmFile) {
        console.error(`file "${baseName}.asm" unknow in ${dir}`);
        process.exit(1);
    }

    await clearBuildArtifacts(dir, outName);

    const asmPath = path.join(dir, asmFile);
    const objPath = path.join(dir, `${outName}.o`);
    const exePath = path.join(dir, outName + (process.platform === 'win32' ? '.exe' : ''));

    const format = process.platform === 'win32' ? 'win32' : 'elf64';
    console.log(`Assembly: ${asmFile} → ${outName}.o`);
    execSync(`nasm -f ${format} "${asmPath}" -o "${objPath}"`, { stdio: 'inherit' });

    if (opts.link) {
        console.log(`ld: ${outName}.o → ${outName}`);
        try {
            if (opts.linkMode === 'gcc') {
                execSync(`gcc -no-pie -nostdlib "${objPath}" -o "${exePath}"`, { stdio: 'inherit' });
            } else {
                if (process.platform === 'win32') {
                    execSync(`ld "${objPath}" -o "${exePath}"`, { stdio: 'inherit' });
                } else {
                    execSync(`/usr/bin/ld -e _start "${objPath}" -o "${exePath}"`, { stdio: 'inherit' });
                }
            }
            console.log(`Successl! File: ${exePath}`);
        } catch (err) {
            console.error('Error ld. Please check:');
            if (opts.linkMode === 'standalone') {
                console.error('  • В коде есть метка _start (а не main)');
                console.error('  • Вы не используете функции libc (printf, exit и т.д.)');
            } else {
                console.error('  • В коде есть main (а не _start)');
                console.error('  • Установлен gcc');
            }
            process.exit(1);
        }
    } else {
        console.log(`object file built: ${objPath}`);
    }
}

main().catch(err => {
    console.error('critical error!', err.message);
    process.exit(1);
});
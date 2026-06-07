/*
 * AI.duino - Code Analyzer Utility
 * Copyright 2025 Monster Maker
 * 
 * Licensed under the Apache License, Version 2.0
 */

"use strict";

/**
 * Extract comprehensive pin configuration from code
 * Analyzes pinMode, digitalWrite, digitalRead, analogWrite, analogRead, interrupts
 * @param {string} code - Source code
 * @returns {Array} Pin configurations with usage details
 */
function extractPinConfiguration(code, t = null) {
    const pins = new Map();
    
    // pinMode declarations
    const pinModeRegex = /pinMode\s*\(\s*(\w+|\d+)\s*,\s*(INPUT|OUTPUT|INPUT_PULLUP)\s*\)/g;
    let match;
    while ((match = pinModeRegex.exec(code)) !== null) {
        const pin = match[1];
        const mode = match[2];
        if (!pins.has(pin)) {
            pins.set(pin, { modes: [], operations: [] });
        }
        pins.get(pin).modes.push(mode);
    }
    
    // digitalWrite operations
    const digitalWriteRegex = /digitalWrite\s*\(\s*(\w+|\d+)\s*,\s*(HIGH|LOW|[01])\s*\)/g;
    while ((match = digitalWriteRegex.exec(code)) !== null) {
        const pin = match[1];
        const value = match[2];
        if (!pins.has(pin)) {
            pins.set(pin, { modes: [`OUTPUT ${t ? t('analyzeCode.inferred') : '(inferred)'}`], operations: [] });
        }
        pins.get(pin).operations.push(`${t ? t('analyzeCode.writes') : 'writes'} ${value}`);
    }
    
    // digitalRead operations
    const digitalReadRegex = /digitalRead\s*\(\s*(\w+|\d+)\s*\)/g;
    while ((match = digitalReadRegex.exec(code)) !== null) {
        const pin = match[1];
        if (!pins.has(pin)) {
            pins.set(pin, { modes: [`INPUT ${t ? t('analyzeCode.inferred') : '(inferred)'}`], operations: [] });
        }
        pins.get(pin).operations.push(t ? t('analyzeCode.readsDigital') : 'reads digital');
    }
    
    // analogWrite (PWM)
    const analogWriteRegex = /analogWrite\s*\(\s*(\w+|\d+)\s*,\s*(\d+|\w+)\s*\)/g;
    while ((match = analogWriteRegex.exec(code)) !== null) {
        const pin = match[1];
        const value = match[2];
        if (!pins.has(pin)) {
            pins.set(pin, { modes: ['PWM'], operations: [] });
        }
        const pwmText = t ? t('analyzeCode.pwmValue').replace('{0}', value) : `PWM (value: ${value})`;
        pins.get(pin).operations.push(pwmText);
    }
    
    // analogRead
    const analogReadRegex = /analogRead\s*\(\s*(\w+|\d+)\s*\)/g;
    while ((match = analogReadRegex.exec(code)) !== null) {
        const pin = match[1];
        if (!pins.has(pin)) {
            pins.set(pin, { modes: ['ANALOG_INPUT'], operations: [] });
        }
        pins.get(pin).operations.push(t ? t('analyzeCode.readsAnalog') + ' (0-1023)' : 'reads analog (0-1023)');
    }
    
    // Interrupts
    const interruptRegex = /attachInterrupt\s*\(\s*digitalPinToInterrupt\s*\(\s*(\w+|\d+)\s*\)\s*,\s*(\w+)\s*,\s*(\w+)\s*\)/g;
    while ((match = interruptRegex.exec(code)) !== null) {
        const pin = match[1];
        const isr = match[2];
        const mode = match[3];
        if (!pins.has(pin)) {
            pins.set(pin, { modes: ['INTERRUPT'], operations: [] });
        }
        const interruptText = t 
            ? t('analyzeCode.interruptMode').replace('{0}', mode).replace('{1}', isr)
            : `interrupt ${mode} → ${isr}()`;
        pins.get(pin).operations.push(interruptText);
    }
    
    // Servo
    const servoAttachRegex = /(\w+)\.attach\s*\(\s*(\w+|\d+)\s*\)/g;
    while ((match = servoAttachRegex.exec(code)) !== null) {
        const servoName = match[1];
        const pin = match[2];
        // Only if it looks like a servo object
        if (code.includes('Servo') && code.includes(servoName)) {
            if (!pins.has(pin)) {
                pins.set(pin, { modes: ['SERVO'], operations: [] });
            }
            const servoText = t 
                ? t('analyzeCode.servoAttached').replace('{0}', servoName)
                : `servo (${servoName})`;
            pins.get(pin).operations.push(servoText);
        }
    }
    
    // Format output
    const result = [];
    for (const [pin, data] of pins.entries()) {
        const modes = [...new Set(data.modes)].join(', ');
        const ops = [...new Set(data.operations)].join(', ');
        const opsText = ops ? ` → ${ops}` : '';
        const pinText = t 
            ? t('analyzeCode.pinFormat').replace('{0}', pin).replace('{1}', modes).replace('{2}', opsText)
            : `Pin ${pin}: ${modes}${opsText}`;
        result.push(pinText);
    }
    
    return result;
}

/**
 * Extract direct port manipulation (DDRx, PORTx, PINx)
 * @param {string} code - Source code
 * @param {Function} t - Translation function (optional)
 * @returns {Array} Port manipulation entries
 */
function extractPortManipulation(code, t = null) {
    const ports = new Map();
    
    // DDRx = ... (complete port assignment)
    const ddrAssignRegex = /DDR([A-L])\s*=\s*(0x[0-9A-Fa-f]+|0b[01]+|\d+)/g;
    let match;
    while ((match = ddrAssignRegex.exec(code)) !== null) {
        const port = match[1];
        const value = match[2];
        const key = `DDR${port}`;
        if (!ports.has(key)) {
            ports.set(key, []);
        }
        ports.get(key).push(`DDR${port} = ${value}`);
    }
    
    // DDRx |= (1 << BIT) - set bit as output
    const ddrSetRegex = /DDR([A-L])\s*\|=\s*\(?\s*1\s*<<\s*(\w+)\s*\)?/g;
    while ((match = ddrSetRegex.exec(code)) !== null) {
        const port = match[1];
        const bit = match[2];
        const text = t 
            ? `PORT${port} Bit ${bit} → OUTPUT (${t('analyzeCode.inferred')})`
        : `Port ${port} Bit ${bit} → OUTPUT (inferred)`;
        const key = `PORT${port}`;
        if (!ports.has(key)) {
            ports.set(key, []);
        }
        ports.get(key).push(text);
    }
    
    // DDRx &= ~(1 << BIT) - set bit as input
    const ddrClearRegex = /DDR([A-L])\s*&=\s*~\s*\(?\s*1\s*<<\s*(\w+)\s*\)?/g;
    while ((match = ddrClearRegex.exec(code)) !== null) {
        const port = match[1];
        const bit = match[2];
        const text = t 
            ? `PORT${port} Bit ${bit} → INPUT (${t('analyzeCode.inferred')})`
            : `Port ${port} Bit ${bit} → INPUT (inferred)`;
        const key = `PORT${port}`;
        if (!ports.has(key)) {
            ports.set(key, []);
        }
        ports.get(key).push(text);
    }
    
    // PORTx |= (1 << BIT) - set output high
    const portSetRegex = /PORT([A-L])\s*\|=\s*\(?\s*1\s*<<\s*(\w+)\s*\)?/g;
    while ((match = portSetRegex.exec(code)) !== null) {
        const port = match[1];
        const bit = match[2];
        const text = t 
            ? `PORT${port} Bit ${bit} → ${t('analyzeCode.portWrite')} HIGH`
            : `Port ${port} Bit ${bit} → writes HIGH`;
        const key = `PORT${port}`;
        if (!ports.has(key)) {
            ports.set(key, []);
        }
        ports.get(key).push(text);
    }
    
    // PORTx &= ~(1 << BIT) - set output low
    const portClearRegex = /PORT([A-L])\s*&=\s*~\s*\(?\s*1\s*<<\s*(\w+)\s*\)?/g;
    while ((match = portClearRegex.exec(code)) !== null) {
        const port = match[1];
        const bit = match[2];
        const text = t 
            ? `PORT${port} Bit ${bit} → ${t('analyzeCode.portWrite')} LOW`
            : `Port ${port} Bit ${bit} → writes LOW`;
        const key = `PORT${port}`;
        if (!ports.has(key)) {
            ports.set(key, []);
        }
        ports.get(key).push(text);
    }
    
    // PORTx ^= (1 << BIT) - toggle output
    const portToggleRegex = /PORT([A-L])\s*\^=\s*\(?\s*1\s*<<\s*(\w+)\s*\)?/g;
    while ((match = portToggleRegex.exec(code)) !== null) {
        const port = match[1];
        const bit = match[2];
        const text = `PORT${port} Bit ${bit} → toggle`;
        const key = `PORT${port}`;
        if (!ports.has(key)) {
            ports.set(key, []);
        }
        ports.get(key).push(text);
    }
    
    // PINx & (1 << BIT) - read input
    const pinReadRegex = /PIN([A-L])\s*&\s*\(?\s*1\s*<<\s*(\w+)\s*\)?/g;
    while ((match = pinReadRegex.exec(code)) !== null) {
        const port = match[1];
        const bit = match[2];
        const text = t 
            ? `PORT${port} Bit ${bit} → ${t('analyzeCode.portRead')}`
            : `Port ${port} Bit ${bit} → reads`;
        const key = `PORT${port}`;
        if (!ports.has(key)) {
            ports.set(key, []);
        }
        ports.get(key).push(text);
    }
    
    // Format output
    const result = [];
    for (const [port, operations] of ports.entries()) {
        const ops = [...new Set(operations)].join(', ');
        result.push(`${port}: ${ops}`);
    }
    
    return result;
}

/**
 * Extract library includes from code
 * @param {string} code - Source code
 * @returns {Array} Library names
 */
function extractLibraries(code) {
    const includeRegex = /#include\s+[<"]([^>"]+)[>"]/g;
    const libraries = [];
    let match;

    while ((match = includeRegex.exec(code)) !== null) {
        libraries.push(match[1]);
    }

    return libraries;
}

/**
 * Extract function signatures from code
 * @param {string} code - Source code
 * @returns {Array} Function signatures
 */
function extractFunctionSignatures(code) {
    const functionRegex = /^\s*(?:void|int|float|double|bool|String|char|long|byte|unsigned\s+\w+)\s+(\w+)\s*\([^)]*\)/gm;
    const functions = [];
    let match;

    while ((match = functionRegex.exec(code)) !== null) {
        functions.push(match[0].trim());
    }

    return functions;
}

/**
 * Extract constants and defines
 * @param {string} code - Source code
 * @returns {Array} Constants with values
 */
function extractConstants(code) {
    const constants = [];
    
    // #define macros
    const defineRegex = /#define\s+(\w+)\s+(.+?)(?:\r?\n|$)/g;
    let match;
    while ((match = defineRegex.exec(code)) !== null) {
        constants.push(`#define ${match[1]} ${match[2].trim()}`);
    }
    
    // const declarations
    const constRegex = /const\s+(\w+)\s+(\w+)\s*=\s*([^;]+);/g;
    while ((match = constRegex.exec(code)) !== null) {
        constants.push(`const ${match[1]} ${match[2]} = ${match[3].trim()}`);
    }
    
    // constexpr (C++11+)
    const constexprRegex = /constexpr\s+(\w+)\s+(\w+)\s*=\s*([^;]+);/g;
    while ((match = constexprRegex.exec(code)) !== null) {
        constants.push(`constexpr ${match[1]} ${match[2]} = ${match[3].trim()}`);
    }
    
    return constants;
}

/**
 * Extract global variables
 * @param {string} code - Source code
 * @returns {Array} Global variable declarations
 */
function extractGlobalVariables(code) {
    const globals = [];
    
    // Remove function bodies to avoid local variables
    let cleanCode = code.replace(/\{[^{}]*\}/g, '');
    
    // Match global variable declarations (outside functions)
    const globalRegex = /^(?!#|\/\/)(\w+(?:\s+\w+)?)\s+(\w+)(?:\s*=\s*[^;]+)?;/gm;
    let match;
    
    while ((match = globalRegex.exec(cleanCode)) !== null) {
        const type = match[1].trim();
        const name = match[2].trim();
        
        // Filter out function declarations and common non-variables
        if (!type.includes('(') && !['if', 'for', 'while', 'return', 'case'].includes(type)) {
            globals.push(`${type} ${name}`);
        }
    }
    
    return globals;
}

/**
 * Extract data structures (struct, class, enum)
 * @param {string} code - Source code
 * @returns {Array} Data structure definitions
 */
function extractDataStructures(code) {
    const structures = [];
    
    // struct definitions
    const structRegex = /struct\s+(\w+)\s*\{[^}]*\}/g;
    let match;
    while ((match = structRegex.exec(code)) !== null) {
        structures.push(match[0].trim());
    }
    
    // class definitions (simple - just signature)
    const classRegex = /class\s+(\w+)(?:\s*:\s*public\s+\w+)?\s*\{/g;
    while ((match = classRegex.exec(code)) !== null) {
        structures.push(`class ${match[1]} { ... }`);
    }
    
    // enum definitions
    const enumRegex = /enum\s+(?:class\s+)?(\w+)\s*\{[^}]*\}/g;
    while ((match = enumRegex.exec(code)) !== null) {
        structures.push(match[0].trim());
    }
    
    return structures;
}

/**
 * Get project file structure
 * @param {Array} files - Array of file paths
 * @returns {string} Formatted file tree
 */
function formatFileStructure(files) {
    if (!files || files.length === 0) return '';
    
    const tree = [];
    files.forEach(file => {
        const parts = file.split('/');
        const indent = '  '.repeat(parts.length - 1);
        tree.push(`${indent}${parts[parts.length - 1]}`);
    });
    
    return tree.join('\n');
}

module.exports = {
    extractPinConfiguration,
    extractLibraries,
    extractFunctionSignatures,
    extractConstants,
    extractGlobalVariables,
    extractDataStructures,
    extractPortManipulation,
    formatFileStructure
};

/**
 * Configuration for SRAM PUF Authentication Server
 */
module.exports = {
    // Server port
    port: process.env.PORT || 8080,

    // API Keys Registry
    apiKeys: {
        '260f1105f729c7afcd71b62eacb5c0b94fcca4c16f6be36c4b2a297a975d358a': {
            deviceId: 'ESP32_001',
            description: 'Test Device 1',
            registered: '2024-01-01'
        },
        'de8b88829965fa9a3615d11b0703c42697ef5bea7ea341983b317ef0e482a852': {
            deviceId: 'ESP32_001',
            description: 'Test Device 1 (Alternative measurement)',
            registered: '2024-01-01'
        },
        '0f569619e8886a2a6f647aacd1d5ce19ff72faa7e47c8125126fe3b104970b3f': {
            deviceId: 'ESP32_ECE580_DEMO',
            description: 'Divija is so cool',
            registered: '2026-03-27'
        },
    },  // <-- closes apiKeys

    // Security settings
    security: {
        debugEndpoints: true,
        logAttempts: true
    }
};
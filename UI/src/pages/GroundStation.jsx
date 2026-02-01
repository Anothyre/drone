import React, { useState, useCallback, useRef, useEffect } from 'react';
import { Button } from "@/components/ui/button";
import { Play, Square, RotateCcw, Radio } from 'lucide-react';

import { encodeControlPacket, decodePacket } from '../components/drone/DronePacketEncoder';
import { useInputHandler, FLIGHT_MODES } from '../components/drone/InputHandler';
import ControlVisualizer from '../components/drone/ControlVisualizer';
import DebugConsole, { createLogEntry } from '../components/drone/DebugConsole';
import ConnectionStatus from '../components/drone/ConnectionStatus';
import SettingsPanel from '../components/drone/SettingsPanel';
import KeyboardHelp from '../components/drone/KeyboardHelp';

/**
 * Main Ground Station Control Page
 * 
 * NOTE: Browser limitations prevent direct UDP socket access.
 * This implementation uses a WebSocket bridge approach.
 * For production, you'd need a small Node.js server as UDP bridge.
 */
export default function GroundStation() {
    // Connection settings
    const [settings, setSettings] = useState({
        targetIP: '192.168.4.1',
        targetPort: 14550,
        sendRate: 50,
        keyboardSensitivity: 1.0,
        gamepadDeadzone: 0.1,
    });

    // State
    const [isActive, setIsActive] = useState(false);
    const [controlState, setControlState] = useState({ x: 0, y: 0, z: 0, mode: 0 });
    const [pressedKeys, setPressedKeys] = useState([]);
    const [gamepadState, setGamepadState] = useState({ connected: false });
    const [logs, setLogs] = useState([]);
    const [stats, setStats] = useState({ packetsSent: 0, packetsPerSecond: 0 });

    // Refs for send loop
    const sendIntervalRef = useRef(null);
    const sequenceRef = useRef(0);
    const packetCountRef = useRef(0);
    const lastSecondRef = useRef(Date.now());

    // Add log entry
    const addLog = useCallback((message, type = 'info') => {
        setLogs(prev => [...prev.slice(-100), createLogEntry(message, type)]);
    }, []);

    // Clear logs
    const clearLogs = useCallback(() => {
        setLogs([]);
    }, []);

    // Input handler
    const { resetControls } = useInputHandler({
        onControlUpdate: setControlState,
        onKeyChange: setPressedKeys,
        onGamepadChange: setGamepadState,
        enabled: isActive,
        keyboardSensitivity: settings.keyboardSensitivity,
        gamepadDeadzone: settings.gamepadDeadzone,
    });

    // Send packet (simulated - in production this would go through WebSocket to UDP bridge)
    const sendPacket = useCallback(() => {
        const packet = encodeControlPacket({
            seq: sequenceRef.current++,
            x: controlState.x,
            y: controlState.y,
            z: controlState.z,
            mode: controlState.mode,
        });

        // Log packet info periodically (every 50 packets)
        if (sequenceRef.current % 50 === 0) {
            const decoded = decodePacket(packet);
            addLog(
                `TX #${decoded.seq}: x=${decoded.x} y=${decoded.y} z=${decoded.z} mode=${decoded.mode}`,
                'packet'
            );
        }

        // Update stats
        packetCountRef.current++;
        const now = Date.now();
        if (now - lastSecondRef.current >= 1000) {
            setStats(prev => ({
                packetsSent: prev.packetsSent + packetCountRef.current,
                packetsPerSecond: packetCountRef.current,
            }));
            packetCountRef.current = 0;
            lastSecondRef.current = now;
        }

        // In production: send packet via WebSocket to UDP bridge
        // ws.send(packet);
        
        return packet;
    }, [controlState, addLog]);

    // Start sending
    const startSending = useCallback(() => {
        if (sendIntervalRef.current) return;

        setIsActive(true);
        sequenceRef.current = 0;
        packetCountRef.current = 0;
        lastSecondRef.current = Date.now();

        addLog(`Started sending to ${settings.targetIP}:${settings.targetPort} @ ${settings.sendRate}Hz`, 'success');

        const intervalMs = 1000 / settings.sendRate;
        sendIntervalRef.current = setInterval(sendPacket, intervalMs);
    }, [settings, sendPacket, addLog]);

    // Stop sending
    const stopSending = useCallback(() => {
        if (sendIntervalRef.current) {
            clearInterval(sendIntervalRef.current);
            sendIntervalRef.current = null;
        }
        setIsActive(false);
        addLog('Stopped sending', 'warning');
    }, [addLog]);

    // Reset everything
    const handleReset = useCallback(() => {
        stopSending();
        resetControls();
        setStats({ packetsSent: 0, packetsPerSecond: 0 });
        sequenceRef.current = 0;
        addLog('Controls reset', 'info');
    }, [stopSending, resetControls, addLog]);

    // Cleanup on unmount
    useEffect(() => {
        return () => {
            if (sendIntervalRef.current) {
                clearInterval(sendIntervalRef.current);
            }
        };
    }, []);

    // Initial log
    useEffect(() => {
        addLog('Ground Station initialized', 'info');
        addLog('Connect to DRONE_FC WiFi (password: drone123)', 'info');
        addLog('Note: Browser UDP requires WebSocket bridge for production', 'warning');
    }, []);

    return (
        <div className="min-h-screen bg-gradient-to-br from-slate-950 via-slate-900 to-slate-950 text-white p-4 md:p-6">
            <div className="max-w-6xl mx-auto space-y-6">
                {/* Header */}
                <div className="flex items-center justify-between">
                    <div className="flex items-center gap-3">
                        <div className="p-2 bg-cyan-500/20 rounded-lg">
                            <Radio className="w-6 h-6 text-cyan-400" />
                        </div>
                        <div>
                            <h1 className="text-xl md:text-2xl font-bold bg-gradient-to-r from-cyan-400 to-emerald-400 bg-clip-text text-transparent">
                                Drone Ground Station
                            </h1>
                            <p className="text-xs text-slate-500">UDP Control Interface</p>
                        </div>
                    </div>

                    {/* Control buttons */}
                    <div className="flex items-center gap-2">
                        <Button
                            variant="outline"
                            size="sm"
                            onClick={handleReset}
                            className="border-slate-700 text-slate-300 hover:bg-slate-800"
                        >
                            <RotateCcw className="w-4 h-4 mr-1" />
                            Reset
                        </Button>
                        
                        {isActive ? (
                            <Button
                                size="sm"
                                onClick={stopSending}
                                className="bg-red-600 hover:bg-red-700 text-white"
                            >
                                <Square className="w-4 h-4 mr-1" />
                                Stop
                            </Button>
                        ) : (
                            <Button
                                size="sm"
                                onClick={startSending}
                                className="bg-emerald-600 hover:bg-emerald-700 text-white"
                            >
                                <Play className="w-4 h-4 mr-1" />
                                Start
                            </Button>
                        )}
                    </div>
                </div>

                {/* Main content grid */}
                <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
                    {/* Left column - Controls visualization */}
                    <div className="lg:col-span-2 space-y-6">
                        {/* Control visualizer */}
                        <div className="bg-slate-800/30 rounded-2xl border border-slate-700/50 p-6">
                            <h2 className="text-sm font-medium text-slate-400 uppercase tracking-wider mb-4">
                                Control Output
                            </h2>
                            <ControlVisualizer
                                x={controlState.x}
                                y={controlState.y}
                                z={controlState.z}
                                mode={controlState.mode}
                                pressedKeys={pressedKeys}
                                gamepadConnected={gamepadState.connected}
                            />
                        </div>

                        {/* Debug console */}
                        <DebugConsole
                            logs={logs}
                            onClear={clearLogs}
                            maxHeight={200}
                        />
                    </div>

                    {/* Right column - Status & Settings */}
                    <div className="space-y-6">
                        {/* Connection status */}
                        <ConnectionStatus
                            isConnected={isActive}
                            isSending={isActive}
                            packetsSent={stats.packetsSent}
                            packetsPerSecond={stats.packetsPerSecond}
                            targetIP={settings.targetIP}
                            targetPort={settings.targetPort}
                        />

                        {/* Settings */}
                        <SettingsPanel
                            settings={settings}
                            onSettingsChange={setSettings}
                            disabled={isActive}
                        />

                        {/* Keyboard help */}
                        <KeyboardHelp />
                    </div>
                </div>

                {/* Footer note */}
                <div className="text-center text-xs text-slate-600 pt-4 border-t border-slate-800">
                    <p>⚠️ Production deployment requires Node.js WebSocket→UDP bridge server</p>
                    <p className="mt-1">Packet format: 23 bytes | Magic: 0x44524F4E | CRC-16-IBM</p>
                </div>
            </div>
        </div>
    );
}
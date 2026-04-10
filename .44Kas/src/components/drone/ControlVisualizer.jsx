import React from 'react';
import { FLIGHT_MODES } from './InputHandler';

/**
 * Visual representation of current control values
 */
export default function ControlVisualizer({ 
    x = 0, 
    y = 0, 
    z = 0, 
    mode = 0,
    pressedKeys = [],
    gamepadConnected = false,
}) {
    // Convert -1 to 1 range to percentage for positioning
    const stickX = ((x + 1) / 2) * 100;
    const stickY = ((1 - y) / 2) * 100; // Invert Y for visual
    const throttleHeight = z * 100;

    return (
        <div className="grid grid-cols-3 gap-6">
            {/* XY Stick Display */}
            <div className="flex flex-col items-center">
                <span className="text-xs text-slate-400 mb-2 uppercase tracking-wider">Stick X/Y</span>
                <div className="relative w-32 h-32 bg-slate-800/50 rounded-xl border border-slate-700/50 overflow-hidden">
                    {/* Grid lines */}
                    <div className="absolute inset-0 flex items-center justify-center">
                        <div className="absolute w-full h-px bg-slate-700/50" />
                        <div className="absolute h-full w-px bg-slate-700/50" />
                    </div>
                    
                    {/* Stick position indicator */}
                    <div 
                        className="absolute w-6 h-6 -ml-3 -mt-3 rounded-full bg-gradient-to-br from-cyan-400 to-cyan-600 shadow-lg shadow-cyan-500/30 transition-all duration-75"
                        style={{ 
                            left: `${stickX}%`, 
                            top: `${stickY}%`,
                        }}
                    >
                        <div className="absolute inset-1 rounded-full bg-cyan-300/30" />
                    </div>
                </div>
                <div className="mt-2 text-xs font-mono text-slate-400">
                    X: <span className="text-cyan-400">{x.toFixed(2)}</span>
                    {' | '}
                    Y: <span className="text-cyan-400">{y.toFixed(2)}</span>
                </div>
            </div>

            {/* Throttle Display */}
            <div className="flex flex-col items-center">
                <span className="text-xs text-slate-400 mb-2 uppercase tracking-wider">Throttle</span>
                <div className="relative w-12 h-32 bg-slate-800/50 rounded-xl border border-slate-700/50 overflow-hidden">
                    {/* Throttle level */}
                    <div 
                        className="absolute bottom-0 left-0 right-0 bg-gradient-to-t from-emerald-500 to-emerald-400 transition-all duration-75"
                        style={{ height: `${throttleHeight}%` }}
                    />
                    
                    {/* Tick marks */}
                    {[25, 50, 75].map(tick => (
                        <div 
                            key={tick}
                            className="absolute left-0 right-0 h-px bg-slate-600/50"
                            style={{ bottom: `${tick}%` }}
                        />
                    ))}
                </div>
                <div className="mt-2 text-xs font-mono text-slate-400">
                    Z: <span className="text-emerald-400">{(z * 100).toFixed(0)}%</span>
                </div>
            </div>

            {/* Mode & Status */}
            <div className="flex flex-col items-center">
                <span className="text-xs text-slate-400 mb-2 uppercase tracking-wider">Mode</span>
                <div className="w-32 h-32 bg-slate-800/50 rounded-xl border border-slate-700/50 p-3 flex flex-col justify-between">
                    {/* Mode indicator */}
                    <div className="text-center">
                        <div className="text-3xl font-bold text-amber-400">{mode}</div>
                        <div className="text-xs text-slate-400 mt-1">
                            {FLIGHT_MODES[mode] || 'UNKNOWN'}
                        </div>
                    </div>
                    
                    {/* Gamepad status */}
                    <div className={`flex items-center justify-center gap-2 text-xs ${gamepadConnected ? 'text-emerald-400' : 'text-slate-500'}`}>
                        <div className={`w-2 h-2 rounded-full ${gamepadConnected ? 'bg-emerald-400' : 'bg-slate-600'}`} />
                        {gamepadConnected ? 'Gamepad' : 'No Gamepad'}
                    </div>
                </div>
            </div>

            {/* Pressed Keys Display */}
            <div className="col-span-3 mt-2">
                <span className="text-xs text-slate-400 uppercase tracking-wider">Active Keys</span>
                <div className="flex flex-wrap gap-2 mt-2 min-h-[32px]">
                    {pressedKeys.length === 0 ? (
                        <span className="text-xs text-slate-600 italic">No keys pressed</span>
                    ) : (
                        pressedKeys.map((key, i) => (
                            <span 
                                key={i}
                                className="px-2 py-1 bg-cyan-500/20 border border-cyan-500/30 rounded text-cyan-400 text-xs font-mono"
                            >
                                {key}
                            </span>
                        ))
                    )}
                </div>
            </div>
        </div>
    );
}
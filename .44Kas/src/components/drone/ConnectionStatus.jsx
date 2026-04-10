import React from 'react';
import { Wifi, WifiOff, Send, AlertTriangle } from 'lucide-react';

/**
 * Connection status indicator
 */
export default function ConnectionStatus({ 
    isConnected = false,
    isSending = false,
    packetsSent = 0,
    packetsPerSecond = 0,
    lastError = null,
    targetIP = '',
    targetPort = 0,
}) {
    return (
        <div className="bg-slate-800/50 rounded-xl border border-slate-700/50 p-4">
            <div className="flex items-center justify-between mb-4">
                <div className="flex items-center gap-3">
                    {isConnected ? (
                        <div className="p-2 bg-emerald-500/20 rounded-lg">
                            <Wifi className="w-5 h-5 text-emerald-400" />
                        </div>
                    ) : (
                        <div className="p-2 bg-slate-700/50 rounded-lg">
                            <WifiOff className="w-5 h-5 text-slate-500" />
                        </div>
                    )}
                    <div>
                        <div className="text-sm font-medium text-slate-200">
                            {isConnected ? 'Connected' : 'Disconnected'}
                        </div>
                        <div className="text-xs text-slate-500">
                            {targetIP}:{targetPort}
                        </div>
                    </div>
                </div>

                {/* Send indicator */}
                {isSending && (
                    <div className="flex items-center gap-2 text-cyan-400">
                        <Send className="w-4 h-4 animate-pulse" />
                        <span className="text-xs font-mono">{packetsPerSecond} Hz</span>
                    </div>
                )}
            </div>

            {/* Stats */}
            <div className="grid grid-cols-2 gap-4 text-center">
                <div className="bg-slate-900/50 rounded-lg p-2">
                    <div className="text-lg font-bold text-cyan-400 font-mono">
                        {packetsSent.toLocaleString()}
                    </div>
                    <div className="text-xs text-slate-500 uppercase">Packets Sent</div>
                </div>
                <div className="bg-slate-900/50 rounded-lg p-2">
                    <div className="text-lg font-bold text-emerald-400 font-mono">
                        {packetsPerSecond}
                    </div>
                    <div className="text-xs text-slate-500 uppercase">Packets/sec</div>
                </div>
            </div>

            {/* Error display */}
            {lastError && (
                <div className="mt-3 flex items-center gap-2 text-xs text-red-400 bg-red-500/10 rounded-lg p-2">
                    <AlertTriangle className="w-4 h-4 flex-shrink-0" />
                    <span className="truncate">{lastError}</span>
                </div>
            )}
        </div>
    );
}
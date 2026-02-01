import React, { useRef, useEffect } from 'react';
import { ChevronDown, Trash2 } from 'lucide-react';

/**
 * Debug console for displaying logs and packet information
 */
export default function DebugConsole({ 
    logs = [], 
    onClear,
    maxHeight = 200,
    autoScroll = true,
}) {
    const scrollRef = useRef(null);

    useEffect(() => {
        if (autoScroll && scrollRef.current) {
            scrollRef.current.scrollTop = scrollRef.current.scrollHeight;
        }
    }, [logs, autoScroll]);

    const getLogColor = (type) => {
        switch (type) {
            case 'error': return 'text-red-400';
            case 'warning': return 'text-amber-400';
            case 'success': return 'text-emerald-400';
            case 'packet': return 'text-cyan-400';
            case 'info': return 'text-blue-400';
            default: return 'text-slate-300';
        }
    };

    const getLogIcon = (type) => {
        switch (type) {
            case 'error': return '✖';
            case 'warning': return '⚠';
            case 'success': return '✓';
            case 'packet': return '→';
            case 'info': return 'ℹ';
            default: return '•';
        }
    };

    return (
        <div className="bg-slate-900/80 rounded-xl border border-slate-700/50 overflow-hidden">
            {/* Header */}
            <div className="flex items-center justify-between px-4 py-2 border-b border-slate-700/50 bg-slate-800/30">
                <span className="text-xs text-slate-400 uppercase tracking-wider font-medium">
                    Debug Console
                </span>
                <div className="flex items-center gap-2">
                    <span className="text-xs text-slate-500">{logs.length} entries</span>
                    {onClear && (
                        <button 
                            onClick={onClear}
                            className="p-1 text-slate-500 hover:text-slate-300 transition-colors"
                            title="Clear console"
                        >
                            <Trash2 className="w-3.5 h-3.5" />
                        </button>
                    )}
                </div>
            </div>

            {/* Log entries */}
            <div 
                ref={scrollRef}
                className="overflow-y-auto font-mono text-xs p-3 space-y-1"
                style={{ maxHeight }}
            >
                {logs.length === 0 ? (
                    <div className="text-slate-600 text-center py-4 italic">
                        No logs yet...
                    </div>
                ) : (
                    logs.map((log, i) => (
                        <div 
                            key={i}
                            className={`flex items-start gap-2 ${getLogColor(log.type)}`}
                        >
                            <span className="w-4 text-center opacity-60">{getLogIcon(log.type)}</span>
                            <span className="text-slate-500 flex-shrink-0">
                                {log.timestamp}
                            </span>
                            <span className="flex-1 break-all">
                                {log.message}
                            </span>
                        </div>
                    ))
                )}
            </div>
        </div>
    );
}

/**
 * Create a log entry
 */
export function createLogEntry(message, type = 'info') {
    const now = new Date();
    const timestamp = now.toLocaleTimeString('en-US', { 
        hour12: false,
        hour: '2-digit',
        minute: '2-digit',
        second: '2-digit',
    }) + '.' + String(now.getMilliseconds()).padStart(3, '0');
    
    return { timestamp, message, type };
}
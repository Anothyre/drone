import React from 'react';
import { Keyboard, Gamepad2 } from 'lucide-react';

/**
 * Keyboard and gamepad control reference
 */
export default function KeyboardHelp() {
    const keyGroups = [
        {
            title: 'Movement',
            keys: [
                { key: 'W / ↑', action: 'Pitch Forward' },
                { key: 'S / ↓', action: 'Pitch Back' },
                { key: 'A / ←', action: 'Roll Left' },
                { key: 'D / →', action: 'Roll Right' },
            ]
        },
        {
            title: 'Throttle',
            keys: [
                { key: 'Space', action: 'Throttle Up (fast)' },
                { key: 'Shift', action: 'Throttle Down (fast)' },
                { key: 'E', action: 'Throttle Up (fine)' },
                { key: 'Q', action: 'Throttle Down (fine)' },
            ]
        },
        {
            title: 'Modes',
            keys: [
                { key: '1', action: 'Stabilize' },
                { key: '2', action: 'Alt Hold' },
                { key: '3', action: 'Loiter' },
                { key: '4', action: 'RTL' },
                { key: '5', action: 'Land' },
            ]
        },
        {
            title: 'Emergency',
            keys: [
                { key: 'Esc', action: 'Zero all controls' },
            ]
        },
    ];

    return (
        <div className="bg-slate-800/50 rounded-xl border border-slate-700/50 p-4">
            <div className="flex items-center gap-2 mb-4">
                <Keyboard className="w-4 h-4 text-slate-400" />
                <span className="text-sm font-medium text-slate-200">Controls</span>
            </div>

            <div className="grid grid-cols-2 gap-4">
                {keyGroups.map((group) => (
                    <div key={group.title}>
                        <h4 className="text-xs text-slate-500 uppercase tracking-wider mb-2">
                            {group.title}
                        </h4>
                        <div className="space-y-1">
                            {group.keys.map((item) => (
                                <div 
                                    key={item.key}
                                    className="flex items-center gap-2 text-xs"
                                >
                                    <kbd className="px-1.5 py-0.5 bg-slate-700/50 rounded text-slate-300 font-mono min-w-[40px] text-center">
                                        {item.key}
                                    </kbd>
                                    <span className="text-slate-400">{item.action}</span>
                                </div>
                            ))}
                        </div>
                    </div>
                ))}
            </div>

            {/* Gamepad info */}
            <div className="mt-4 pt-4 border-t border-slate-700/50">
                <div className="flex items-center gap-2 mb-2">
                    <Gamepad2 className="w-4 h-4 text-slate-400" />
                    <span className="text-xs text-slate-400">Xbox Controller</span>
                </div>
                <div className="grid grid-cols-2 gap-2 text-xs">
                    <div className="text-slate-500">Left Stick → X/Y</div>
                    <div className="text-slate-500">Triggers → Throttle</div>
                    <div className="text-slate-500">A/B/X/Y → Modes 0-3</div>
                </div>
            </div>
        </div>
    );
}
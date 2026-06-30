import React from 'react';
import { Input } from "@/components/ui/input";
import { Label } from "@/components/ui/label";
import { Slider } from "@/components/ui/slider";
import { Settings } from 'lucide-react';

/**
 * Configuration panel for drone connection settings
 */
export default function SettingsPanel({ 
    settings,
    onSettingsChange,
    disabled = false,
}) {
    const handleChange = (key, value) => {
        onSettingsChange({ ...settings, [key]: value });
    };

    return (
        <div className="bg-slate-800/50 rounded-xl border border-slate-700/50 p-4">
            <div className="flex items-center gap-2 mb-4">
                <Settings className="w-4 h-4 text-slate-400" />
                <span className="text-sm font-medium text-slate-200">Configuration</span>
            </div>

            <div className="space-y-4">
                {/* Target IP */}
                <div className="space-y-1.5">
                    <Label className="text-xs text-slate-400">Target IP</Label>
                    <Input
                        value={settings.targetIP}
                        onChange={(e) => handleChange('targetIP', e.target.value)}
                        disabled={disabled}
                        className="bg-slate-900/50 border-slate-700 text-slate-200 font-mono text-sm h-9"
                        placeholder="192.168.4.1"
                    />
                </div>

                {/* Target Port */}
                <div className="space-y-1.5">
                    <Label className="text-xs text-slate-400">Control Port</Label>
                    <Input
                        type="number"
                        value={settings.targetPort}
                        onChange={(e) => handleChange('targetPort', parseInt(e.target.value) || 0)}
                        disabled={disabled}
                        className="bg-slate-900/50 border-slate-700 text-slate-200 font-mono text-sm h-9"
                        placeholder="14550"
                    />
                </div>

                {/* Send Rate */}
                <div className="space-y-1.5">
                    <div className="flex items-center justify-between">
                        <Label className="text-xs text-slate-400">Send Rate</Label>
                        <span className="text-xs text-cyan-400 font-mono">{settings.sendRate} Hz</span>
                    </div>
                    <Slider
                        value={[settings.sendRate]}
                        onValueChange={([value]) => handleChange('sendRate', value)}
                        min={10}
                        max={200}
                        step={10}
                        disabled={disabled}
                        className="py-2"
                    />
                </div>

                {/* Keyboard Sensitivity */}
                <div className="space-y-1.5">
                    <div className="flex items-center justify-between">
                        <Label className="text-xs text-slate-400">Keyboard Sensitivity</Label>
                        <span className="text-xs text-cyan-400 font-mono">{settings.keyboardSensitivity.toFixed(1)}</span>
                    </div>
                    <Slider
                        value={[settings.keyboardSensitivity * 10]}
                        onValueChange={([value]) => handleChange('keyboardSensitivity', value / 10)}
                        min={1}
                        max={20}
                        step={1}
                        disabled={disabled}
                        className="py-2"
                    />
                </div>

                {/* Gamepad Deadzone */}
                <div className="space-y-1.5">
                    <div className="flex items-center justify-between">
                        <Label className="text-xs text-slate-400">Gamepad Deadzone</Label>
                        <span className="text-xs text-cyan-400 font-mono">{(settings.gamepadDeadzone * 100).toFixed(0)}%</span>
                    </div>
                    <Slider
                        value={[settings.gamepadDeadzone * 100]}
                        onValueChange={([value]) => handleChange('gamepadDeadzone', value / 100)}
                        min={0}
                        max={30}
                        step={1}
                        disabled={disabled}
                        className="py-2"
                    />
                </div>
            </div>
        </div>
    );
}
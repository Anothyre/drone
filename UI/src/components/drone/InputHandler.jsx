import { useEffect, useCallback, useRef } from 'react';

/**
 * Default keyboard mappings
 * Keys map to control adjustments
 */
export const DEFAULT_KEY_MAPPINGS = {
    // X-axis (roll/strafe)
    'KeyA': { axis: 'x', value: -1, label: 'A' },
    'KeyD': { axis: 'x', value: 1, label: 'D' },
    'ArrowLeft': { axis: 'x', value: -1, label: '←' },
    'ArrowRight': { axis: 'x', value: 1, label: '→' },
    
    // Y-axis (pitch/forward-back)
    'KeyW': { axis: 'y', value: 1, label: 'W' },
    'KeyS': { axis: 'y', value: -1, label: 'S' },
    'ArrowUp': { axis: 'y', value: 1, label: '↑' },
    'ArrowDown': { axis: 'y', value: -1, label: '↓' },
    
    // Z-axis (throttle)
    'Space': { axis: 'z', value: 0.1, label: 'Space' },
    'ShiftLeft': { axis: 'z', value: -0.1, label: 'Shift' },
    'KeyQ': { axis: 'z', value: -0.05, label: 'Q' },
    'KeyE': { axis: 'z', value: 0.05, label: 'E' },
    
    // Flight modes
    'Digit1': { axis: 'mode', value: 0, label: '1' },
    'Digit2': { axis: 'mode', value: 1, label: '2' },
    'Digit3': { axis: 'mode', value: 2, label: '3' },
    'Digit4': { axis: 'mode', value: 3, label: '4' },
    'Digit5': { axis: 'mode', value: 4, label: '5' },
    
    // Emergency
    'Escape': { axis: 'emergency', value: true, label: 'Esc' },
};

export const FLIGHT_MODES = {
    0: 'STABILIZE',
    1: 'ALT_HOLD',
    2: 'LOITER',
    3: 'RTL',
    4: 'LAND',
};

/**
 * Custom hook for handling keyboard and gamepad input
 */
export function useInputHandler({
    onControlUpdate,
    onKeyChange,
    onGamepadChange,
    keyMappings = DEFAULT_KEY_MAPPINGS,
    enabled = true,
    gamepadDeadzone = 0.1,
    keyboardSensitivity = 1.0,
}) {
    const pressedKeysRef = useRef(new Set());
    const controlStateRef = useRef({ x: 0, y: 0, z: 0, mode: 0 });
    const gamepadStateRef = useRef({ connected: false, axes: [0, 0, 0, 0], buttons: [] });
    const animationFrameRef = useRef(null);

    // Clamp value between min and max
    const clamp = (value, min, max) => Math.max(min, Math.min(max, value));

    // Apply deadzone to axis value
    const applyDeadzone = (value, deadzone) => {
        if (Math.abs(value) < deadzone) return 0;
        const sign = value > 0 ? 1 : -1;
        return sign * ((Math.abs(value) - deadzone) / (1 - deadzone));
    };

    // Handle keyboard input
    const handleKeyDown = useCallback((e) => {
        if (!enabled) return;
        
        const mapping = keyMappings[e.code];
        if (!mapping) return;
        
        e.preventDefault();
        
        if (!pressedKeysRef.current.has(e.code)) {
            pressedKeysRef.current.add(e.code);
            onKeyChange?.(Array.from(pressedKeysRef.current).map(k => keyMappings[k]?.label || k));
        }
        
        if (mapping.axis === 'mode') {
            controlStateRef.current.mode = mapping.value;
        } else if (mapping.axis === 'emergency') {
            controlStateRef.current = { x: 0, y: 0, z: 0, mode: controlStateRef.current.mode };
        }
    }, [enabled, keyMappings, onKeyChange]);

    const handleKeyUp = useCallback((e) => {
        if (!enabled) return;
        
        pressedKeysRef.current.delete(e.code);
        onKeyChange?.(Array.from(pressedKeysRef.current).map(k => keyMappings[k]?.label || k));
    }, [enabled, keyMappings, onKeyChange]);

    // Calculate control values from pressed keys
    const calculateKeyboardControls = useCallback(() => {
        let x = 0, y = 0, zDelta = 0;
        
        for (const code of pressedKeysRef.current) {
            const mapping = keyMappings[code];
            if (!mapping) continue;
            
            switch (mapping.axis) {
                case 'x': x += mapping.value; break;
                case 'y': y += mapping.value; break;
                case 'z': zDelta += mapping.value; break;
            }
        }
        
        return {
            x: clamp(x * keyboardSensitivity, -1, 1),
            y: clamp(y * keyboardSensitivity, -1, 1),
            zDelta: zDelta * keyboardSensitivity,
        };
    }, [keyMappings, keyboardSensitivity]);

    // Poll gamepad state
    const pollGamepad = useCallback(() => {
        const gamepads = navigator.getGamepads?.() || [];
        const gamepad = gamepads[0]; // Use first connected gamepad
        
        if (gamepad) {
            const axes = gamepad.axes.map(a => applyDeadzone(a, gamepadDeadzone));
            const buttons = gamepad.buttons.map(b => b.pressed);
            
            gamepadStateRef.current = {
                connected: true,
                id: gamepad.id,
                axes,
                buttons,
            };
            
            onGamepadChange?.(gamepadStateRef.current);
        } else if (gamepadStateRef.current.connected) {
            gamepadStateRef.current = { connected: false, axes: [0, 0, 0, 0], buttons: [] };
            onGamepadChange?.(gamepadStateRef.current);
        }
        
        return gamepadStateRef.current;
    }, [gamepadDeadzone, onGamepadChange]);

    // Main update loop
    const updateLoop = useCallback(() => {
        if (!enabled) {
            animationFrameRef.current = requestAnimationFrame(updateLoop);
            return;
        }
        
        // Get keyboard controls
        const keyboard = calculateKeyboardControls();
        
        // Get gamepad controls
        const gamepad = pollGamepad();
        
        // Combine inputs (gamepad takes priority when active)
        let x = keyboard.x;
        let y = keyboard.y;
        let z = controlStateRef.current.z + keyboard.zDelta * 0.016; // ~60fps delta
        
        if (gamepad.connected && gamepad.axes.length >= 4) {
            // Xbox controller mapping:
            // Left stick X (axis 0) -> roll/x
            // Left stick Y (axis 1) -> pitch/y (inverted)
            // Right stick Y (axis 3) or triggers -> throttle
            if (Math.abs(gamepad.axes[0]) > 0) x = gamepad.axes[0];
            if (Math.abs(gamepad.axes[1]) > 0) y = -gamepad.axes[1]; // Invert Y
            
            // Right trigger (button 7) - throttle up
            // Left trigger (button 6) - throttle down
            if (gamepad.buttons[7]) z += 0.02;
            if (gamepad.buttons[6]) z -= 0.02;
            
            // A button (0) = mode 0, B (1) = mode 1, etc.
            for (let i = 0; i < 4; i++) {
                if (gamepad.buttons[i]) {
                    controlStateRef.current.mode = i;
                    break;
                }
            }
        }
        
        // Clamp final values
        x = clamp(x, -1, 1);
        y = clamp(y, -1, 1);
        z = clamp(z, 0, 1);
        
        // Update state and notify
        const newState = {
            x,
            y,
            z,
            mode: controlStateRef.current.mode,
        };
        
        controlStateRef.current = newState;
        onControlUpdate?.(newState);
        
        animationFrameRef.current = requestAnimationFrame(updateLoop);
    }, [enabled, calculateKeyboardControls, pollGamepad, onControlUpdate]);

    // Setup event listeners
    useEffect(() => {
        window.addEventListener('keydown', handleKeyDown);
        window.addEventListener('keyup', handleKeyUp);
        
        // Start update loop
        animationFrameRef.current = requestAnimationFrame(updateLoop);
        
        return () => {
            window.removeEventListener('keydown', handleKeyDown);
            window.removeEventListener('keyup', handleKeyUp);
            cancelAnimationFrame(animationFrameRef.current);
        };
    }, [handleKeyDown, handleKeyUp, updateLoop]);

    // Reset controls function
    const resetControls = useCallback(() => {
        controlStateRef.current = { x: 0, y: 0, z: 0, mode: 0 };
        pressedKeysRef.current.clear();
        onKeyChange?.([]);
        onControlUpdate?.(controlStateRef.current);
    }, [onControlUpdate, onKeyChange]);

    return {
        resetControls,
        getControlState: () => controlStateRef.current,
    };
}
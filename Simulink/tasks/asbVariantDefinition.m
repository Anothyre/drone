%% asbVariantDefinition.m - Variants definiton
% This script initializes the variant objects for the quadcopter model.

% Copyright 2017-2018 The MathWorks, Inc.

% Command
VSS_COMMAND_SIGEDIT = Simulink.VariantExpression('VSS_COMMAND==0');
VSS_COMMAND_JOYSTICK = Simulink.VariantExpression('VSS_COMMAND==1');
VSS_COMMAND_PRESAVED = Simulink.VariantExpression('VSS_COMMAND==2');

% Sensors
VSS_SENSORS_FEEDTHROUGH = Simulink.VariantExpression('VSS_SENSORS==0');
VSS_SENSORS_DYNAMICS = Simulink.VariantExpression('VSS_SENSORS==1');

% Environment
VSS_ENVIRONMENT_CST = Simulink.VariantExpression('VSS_ENVIRONMENT==0');
VSS_ENVIRONMENT_VARIABLE = Simulink.VariantExpression('VSS_ENVIRONMENT==1');

%Visualization
VSS_VISUALIZATION_SCOPES = Simulink.VariantExpression('VSS_VISUALIZATION==0');
VSS_VISUALIZATION_WORKSPACE = Simulink.VariantExpression('VSS_VISUALIZATION==1');
VSS_VISUALIZATION_FLIGHTGEAR = Simulink.VariantExpression('VSS_VISUALIZATION==2');
VSS_VISUALIZATION_SL3D = Simulink.VariantExpression('VSS_VISUALIZATION==3');

% Actuators
VSS_ACTUATORS_FEEDTHROUGH = Simulink.VariantExpression('VSS_ACTUATORS==0');
VSS_ACTUATORS_LINEAR = Simulink.VariantExpression('VSS_ACTUATORS==1');
VSS_ACTUATORS_NONLINEAR = Simulink.VariantExpression('VSS_ACTUATORS==2');

% Vehicle
VSS_VEHICLE_LINEAR = Simulink.VariantExpression('VSS_VEHICLE==0');
VSS_VEHICLE_NONLINEAR = Simulink.VariantExpression('VSS_VEHICLE==1');

# Explore a map with a plane

Allows users to explore a map from the perspective of a flying object.

![Image of Flight Simulator](FlightSim.jpg)

## How to use the sample

1. Open the **FlightSim_lvl**.
2. Click on **ArcGIS Map Actor** in the **Outliner** panel, and set your API key in the **ArcGIS Map Component**.
3. Click on play and enjoy flying around the world.

### Controls

- Keyboard Controls: Spacebar for Acceleration, Q for Yaw Left, E for Yaw Right, W or Mouse Y for Pitch Up, S or Mouse Y for Pitch Down, A or Mouse X for Roll Left, D or Mouse X for Roll Right. 
H can be used to toggle the landing gear.

- Controller Controls: Right Trigger for Acceleration, Left Trigger for Deceleration, Left Stick Left and Right for Yaw, Right Stick Left and Right for Roll, Right Stick up and down for Pitch. Right Thumbstick button to toggle landing gear. 

## How it works

1. Create a `Global` map and enable mesh colliders on the **ArcGIS Map Component**.
2. Set the **Origin Position** for the place where you want to place the runway.
3. Create a runway and add it to the level. In this sample, the runway is created by using a cube and saved as **BP_Runway**.
4. Create a flying object as a **Pawn** and attach the [**ArcGIS Camera Component**](https://developers.arcgis.com/unreal-engine/maps/camera/#arcgis-camera-component) and [**ArcGIS Location Component**](https://developers.arcgis.com/unreal-engine/maps/location-component/).
5. Connect the flying object to the [**Player Controller**](https://docs.unrealengine.com/5.0/en-US/player-controllers-in-unreal-engine/) and place it on the map. In this sample, the flying object is saved as **BP_Ship**.
6. Work on [physics](https://docs.unrealengine.com/5.0/en-US/physics-in-unreal-engine/) from Unreal Engine's feature for the flying object. This sample uses [**Set Physics Linear Velocity**](https://docs.unrealengine.com/5.0/en-US/BlueprintAPI/Physics/SetPhysicsLinearVelocity/) for the speed control and [**Add Torque in Degrees**](https://docs.unrealengine.com/4.26/en-US/BlueprintAPI/Physics/AddTorqueinDegrees/) for the object's roll, pitch, and yaw. If the plane reduces enough its speed, false gravity drags the plane toward Earth.

## Controls
This sample supports keyboard controls and use of an Xbox/Playstation controller
Keyboard Controls: Spacebar for Acceleration, Q for Yaw Left, E for Yaw Right, W or Mouse Y for Pitch Up, S or Mouse Y for Pitch Down, A or Mouse X for Roll Left, D or Mouse X for Roll Right. 
H can be used to toggle the landing gear.
Controller Controls: Right Trigger for Acceleration, Left Trigger for Deceleration, Left Stick Left and Right for Yaw, Right Stick Left and Right for Roll, Right Stick up and down for Pitch. Right Thumbstick button to toggle landing gear. 

## About the data

Building models for San Francisco are loaded from a [3D object scene layer](https://tiles.arcgis.com/tiles/z2tnIkrLQ2BRzr6P/arcgis/rest/services/SanFrancisco_Bldgs/SceneServer) hosted by Esri.

Elevation data is loaded from the [Terrain 3D elevation layer](https://www.arcgis.com/home/item.html?id=7029fb60158543ad845c7e1527af11e4) hosted by Esri.

## Tags

exploration, third person pespective, Flight Simulator

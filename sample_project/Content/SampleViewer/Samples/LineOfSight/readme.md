# Check line of sight

Determine if the line of sight is obstructed by any object in the scene.

![Image of line of sight](LineOfSight.jpg)

## How to use the sample

1. Open **LineOfSight** level.
2. Click on **ArcGISMapActor** in the Outliner.
3. Set your API key in the Details panel.
4. Click play and see the line colors changes to red if there is any object to obstruct the sight.

## How it works

1. Have an ArcGIS Map with the [mesh collider](https://developers.arcgis.com/unreal-engine/maps/mesh-collider/) enabled.
2. Have a camera with the [**ArcGIS Camera Component**](https://developers.arcgis.com/unreal-engine/maps/camera/#arcgis-camera-component) attached for the area.
3. Have an Actor for checking line of sight (**LineOfSignt** Actor) and another for defining the path that the target object is moving along (**WayPoints** Actor).
4. The moiving object's path is defined by [Spline Components](https://docs.unrealengine.com/5.0/en-US/blueprint-spline-components-overview-in-unreal-engine/).
5. The **LineOfSignt** Actor looks for an instance of the **WayPoints** Actor when play begins performs a [line trace](https://docs.unrealengine.com/5.0/en-US/BlueprintAPI/Collision/LineTraceByChannel/) between them at each frame. 
    - Any actors that unintentionally interfere with the trace can be added to the `Actors to Ignore` argument of the line trace in the **LineOfSignt** Actor blueprint. 
    - The visual cue for the line of sight changes its color depending on whether the line trace hits the target object or not.

## About the data

Building models for New York are loaded from a [3D object scene layer](https://tiles.arcgis.com/tiles/z2tnIkrLQ2BRzr6P/arcgis/rest/services/New_York_LoD2_3D_Buildings/SceneServer/layers/0) hosted by Esri.

Elevation data is loaded from the [Terrain 3D layer](https://elevation3d.arcgis.com/arcgis/rest/services/WorldElevation3D/Terrain3D/ImageServer) hosted by Esri.

## Tags

line of sight, raycast, visibility, visibility analysis
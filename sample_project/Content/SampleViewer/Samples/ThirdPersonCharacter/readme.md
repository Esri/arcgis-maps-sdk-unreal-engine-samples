# Third Person Character Controller

Allows users to explore a city/map from the perspective of a Third Person Character.

![Image of Third Person Controller](3rdPersonDemo.png)

## How to use the sample

1. Open the **ThirdPerson** level
2. Click on **ArcGIS Map Actor** in the **Outliner** panel, and set your API key in the **ArcGIS Map Component**.
3. Click play. Use WASD to move and space for jumping.

## How it works

1. Create a map and enable the mesh collider.
2. Add a third-person character to the level and attach the **ArcGIS Camera Component**.
3. Configure control for the third-person character. In this sample, it is saved as **BP_ThirdPersonCharacter**.

## About the data

Building models for San Francisco are loaded from a [3D object scene layer](https://tiles.arcgis.com/tiles/z2tnIkrLQ2BRzr6P/arcgis/rest/services/SanFrancisco_Bldgs/SceneServer) hosted by Esri.

Elevation data is loaded from the [Terrain 3D elevation layer](https://www.arcgis.com/home/item.html?id=7029fb60158543ad845c7e1527af11e4) hosted by Esri.

## Tags

exploration, third person pespective, third person controller

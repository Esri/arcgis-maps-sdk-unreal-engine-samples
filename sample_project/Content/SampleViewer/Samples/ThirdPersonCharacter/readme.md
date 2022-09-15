# Third Person Character Controller

Allows users to explore a city/map from the perspective of a Third Person Character.

![Image of Third Person Controller](3rdPersonDemo.png)

## How to use the sample

1. Set your API Key in the ArcGISMapActor using the World Outliner window (if you are using the SampleViewerLevel you can also set the key through the UI or the level blueprint).
2. Click play. Use WASD to move and space for jumping.

## How it works

1. In your ArcGIS Maps SDK scene, delete ArcGIS Pawn.
2. Turn on mesh collider.
3. Copy and paste BP_ThirdPersonCharacter to your scene. 

## About the data

Building models for San Francisco are loaded from a [3D object scene layer](https://tiles.arcgis.com/tiles/z2tnIkrLQ2BRzr6P/arcgis/rest/services/SanFrancisco_Bldgs/SceneServer) hosted by Esri.

Elevation data is loaded from the [Terrain 3D elevation layer](https://www.arcgis.com/home/item.html?id=7029fb60158543ad845c7e1527af11e4) hosted by Esri.

## Tags

exploration, third person pespective, third person controller

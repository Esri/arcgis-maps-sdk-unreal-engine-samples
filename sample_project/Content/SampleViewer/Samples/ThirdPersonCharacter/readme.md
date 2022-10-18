# Explore with Third Person Character

Allows users to explore a city/map from the perspective of a Third Person Character.

![Image of Third Person Controller](3rdPersonDemo.png)

## How to use the sample

1. Open the **ThirdPerson** level.
2. Click on the **ArcGISMapActor** in the Outliner panel.
3. Set your API key under th **Authentication** section in the Details panel.
4. Click play and move the character by using the WASD keys and the right mouse button. Jump with the space key.

Note: The terrain needs to be loaded before the Third Person Character falls on the ground. Adjust the `Z` Location value of the **BP_ThirdPersonCharacter** in order to gain more time for the terrain to be loaded if it's necessary.


## How it works

1. Set up **ArcGIS Map** for the exploration area.
2. Create a Third Person Character with its control and attach the [**ArcGIS Camera Component**](https://developers.arcgis.com/unreal-engine/maps/camera/#arcgis-camera-component) to the Character Mesh. 
3. Add the Third Person Character to the level.
   - Mesh colliders need to be enabled in the **ArcGIS Map Component**.
   - If you want to place the character on a specific location, attach the [**ArcGIS Location Component**](https://developers.arcgis.com/unreal-engine/maps/location-component/) to specify it.
   - Adjust the `Z` Location value of the character to have enough time to load the terrain.

## About the data

Building models for San Francisco are loaded from a [3D object scene layer](https://tiles.arcgis.com/tiles/z2tnIkrLQ2BRzr6P/arcgis/rest/services/SanFrancisco_Bldgs/SceneServer) hosted by Esri.

Elevation data is loaded from the [Terrain 3D elevation layer](https://www.arcgis.com/home/item.html?id=7029fb60158543ad845c7e1527af11e4) hosted by Esri.

## Tags

exploration, third person perspective, third person controller

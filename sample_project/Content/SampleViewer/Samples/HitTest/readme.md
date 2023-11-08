# Visualize 3DObject ID's

Get the ID for individual buildings in a scene.

![Image of HitTest](HitTest.png)

## How to use the sample (SampleViewer)

1. The SampleViewer Scene should open by default, if it is not open, click the **SampleViewer** scene to open it.
2. Click play.
3. Using the UI, enter an APIKey in the input field to the top left. 
4. Open the **Samples** drop down, and click **HitTest** to open the level.
5. Click a building. The building's ID's will show up on a 3D UI Component.

## How to use the sample (HitTest Scene)

1. Open the **HitTest** level.
2. Click on the **ArcGISMap** Actor and set your API key in the **Details** panel. 
3. Click play.
4. Click a building. The building's ID's will show up on a 3D UI Component.

## How it works

1. Create an ArcGIS Map.
2. Add an **ArcGISPawn** to the scene.
3. Within the Level Blueprint, create an Input mapping. Off the **Pressed** node, drag off and create a **Line Trace by Channel**.
4. Drag off the Exec node, and create an **ArcGISRaycast Hit**.
5. Break the **ArcGISRaycast Hit Result** and drag off the **FeatureID** node.
6. Convert this integer to a string, and connect it to a print node.
7. Enter your **APIKey** in the **APIMapCreator**.
8. Hit play. Once the scene is running click a building. The building's ID's will show up in a debug on screen. 

## About the data

Building models for New York are loaded from a [3D object scene layer](https://tiles.arcgis.com/tiles/z2tnIkrLQ2BRzr6P/arcgis/rest/services/New_York_LoD2_3D_Buildings/SceneServer/layers/0) hosted by Esri.

Elevation data is loaded from the [Terrain 3D elevation layer](https://www.arcgis.com/home/item.html?id=7029fb60158543ad845c7e1527af11e4) hosted by Esri.

## Tags

raycast, visibility

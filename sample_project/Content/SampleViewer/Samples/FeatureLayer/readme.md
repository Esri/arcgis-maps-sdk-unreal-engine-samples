# Query a feature layer

Query objects from the feature layer provided, or provide your own, in order to show them on the map.

![Image of Feature Layer Sample](FeatureLayer.jpg)

## How to Setup (Feature Layer Level)

1. Open the **FeatureLayer_lvl** level.
2. Click on the **ArcGISMapActor** in the Outliner panel.
3. Set your API key under Authentication section in the Details panel.
4. Click play and select a stadium from the list.

## How to Setup (Sample Viewer)

1. Click Play in Unreal Editor.
2. Input your API key under the **API Key Drop down**.
3. Click the **Sample Drop Down** and select **Feature Layer**.

## How to Use

1. Already available is a Feature Layer of trees, hitting the process button in the UI will query the feature layer and create 3D objects in the scene to visualize where the data is. 
2. Clicking on any of these objects under the **Outliner** in unreal to see it's latatude, longitude, and any properties recieved by the query
3. Under the outfields drop down, by default it will be set to "Get All Outfields". Clicking on any of the other outfields will deselect "Get All Outfields" and will only get the one selected. Multi-select is supported.

## How it works

1. Create a new C++ class and make a http request to [query a feature layer](https://developers.arcgis.com/rest/services-reference/enterprise/query-feature-service-.htm). 
2. Create a new Blueprint Actor class
   - Create the event to place the data returned from the feature layer.
   - Create a function to spawn the actor according to the data recieved in teh query.
   - Attach the [**ArcGIS Location Component**](https://developers.arcgis.com/unreal-engine/maps/location-component/) to the static stadium mesh model.
   - Create a function to place the static mesh model on the `geometry`'s `coordinates` location returned by the feature layer query and the height calculated by [raycasts](https://docs.unrealengine.com/5.0/en-US/using-a-single-line-trace-raycast-by-channel-in-unreal-engine/).
3. Create a widget for the viewport so users can select a stadium from the list fed from the feature service.

Note: You can use `UE_LOG` to print log messages in the **Output Log** window and see if you are gathering the data properly from the feature service.

## About the data

Data for Park Trees [Feature Layer](https://services.arcgis.com/V6ZHFr6zdgNZuVG0/ArcGIS/rest/services/ParkTrees/FeatureServer) hosted by Esri. (Format the request URL for the [query](https://services.arcgis.com/V6ZHFr6zdgNZuVG0/ArcGIS/rest/services/ParkTrees/FeatureServer/0/query?f=geojson&where=1=1&outfields=*).)

Elevation data is loaded from the [Terrain 3D elevation layer](https://www.arcgis.com/home/item.html?id=7029fb60158543ad845c7e1527af11e4) hosted by Esri.

## Tags

Feature Layer, Data Collection

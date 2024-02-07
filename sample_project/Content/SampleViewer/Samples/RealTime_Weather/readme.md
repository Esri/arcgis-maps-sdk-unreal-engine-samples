# Query Current Weather Conditions Across the U.S.A

Query Weather Data from the Weather Stations feature layer provided in order to display weather conditions on the map.

![Image of Weather Sample](Weather.png)

## How to Setup (Weather Sample Level)

1. Open the **RealTimeWeather** level.
2. Click on the **ArcGISMapActor** in the Outliner panel.
3. Set your API key under the Authentication section in the Details panel.
4. Click Play and select a city from the list.

## How to Setup (Sample Viewer)

1. Click Play in Unreal Editor.
2. Input your API key under the **API Key Drop-down**.
3. Click the **Sample Drop-Down** and select **Weather Query**.

## How it works

1. Create a new C++ class and make a http request to [query a feature layer](https://developers.arcgis.com/rest/services-reference/enterprise/query-feature-service-.htm). 
2. Create a new Blueprint Actor class
   - Create the event to place the data returned from the Weather Query.
   - Create a function to spawn the actor according to the data received  in the query.
   - Attach the [**ArcGIS Location Component**](https://developers.arcgis.com/unreal-engine/maps/location-component/) to the weather actor blueprint class.
   - Add aditional functionality to spawn/show different **Niagra Particle Effects** based on the current weather data returned from the **Weather Query**.
3. Create a widget for the viewport so users can select a city from the list fed from the feature service.

Note: You can use `UE_LOG` to print log messages in the **Output Log** window and see if you are gathering the data properly from the feature service.

## About the data

Data for Current Weather in the United States [Feature Layer](https://services9.arcgis.com/RHVPKKiFTONKtxq3/ArcGIS/rest/services/NOAA_METAR_current_wind_speed_direction_v1/FeatureServer/0//query?where=COUNTRY+LIKE+%27%25United+States+of+America%27+AND+WEATHER+NOT+IN(%27No+significant+weather+present+at+this+time.%27%2C+%27Automated+observation+with+no+human+augmentation%3B+there+may+or+may+not+be+significant+weather+present+at+this+time.%27)&outFields=*&f=pgeojson) hosted by Esri.
Elevation data is loaded from the [Terrain 3D elevation layer](https://www.arcgis.com/home/item.html?id=7029fb60158543ad845c7e1527af11e4) hosted by Esri.

## Tags

Feature Layer, Data Collection
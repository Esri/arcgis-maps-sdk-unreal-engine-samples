# Find a route

Show a route between two points on a map using Esri's routing service REST API.

![Routing](Routing.jpg)

## How to use the sample

This sample uses Esri's [routing service's REST API](https://developers.arcgis.com/rest/network/api-reference/overview-of-network-analysis-services.htm) to query the closest route along the road network between two points. This service uses routing operations associated with your API Key. You can learn more about [API keys](https://developers.arcgis.com/documentation/mapping-apis-and-services/security/api-keys/) and [Accounts](https://developers.arcgis.com/documentation/mapping-apis-and-services/deployment/accounts/) in the _Mapping APIs and location services_ guide.

1. Open the **Routing** level
2. Click on the **ArcGISMapActor** in the Outliner panel.
3. Set your API key under **Authentication** section in the Details panel.
4. Click play and left-click on two locations on the map while holding shift. The route between the two points will be shown. 

Note: This sample is only set up to work with mouse and keyboard.

## How it works

1. The `HTTPClient` is required for using the [routing service's REST API](https://developers.arcgis.com/rest/network/api-reference/overview-of-network-analysis-services.htm). 
2. Use the [direct request](https://developers.arcgis.com/rest/network/api-reference/route-synchronous-service.htm) to request and to obtain the routing result.
3. Use [Traces](https://docs.unrealengine.com/5.0/en-US/using-a-single-line-trace-raycast-by-channel-in-unreal-engine/) (Raycasts) to determine the elevation at each breadcrumb's position to account for elevation. (Enable the mesh collider from the **ArcGIS Map Component** to use the Raycast.)
4. Use the [Spline meshes](https://docs.unrealengine.com/5.0/en-US/BlueprintAPI/SplineMesh/) to visualize the route segments between pairs of breadcrumbs.


## Tags

routing, raycast, REST API

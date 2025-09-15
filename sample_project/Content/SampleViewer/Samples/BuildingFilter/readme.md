# Filter building scene layers

Allows users to toggle the visibility of different levels, construction phases, disciplines, and categories within a building scene layer. This sample shows how to explore building scene layers and apply filters using different criteria.

![image](https://github.com/user-attachments/assets/f35d80c0-4d71-4af6-b462-44ef3a5d2d66)

## How to use the sample (SampleViewer)
1. The SampleViewer scene should open by default. If it is not open, click the **SampleViewer** scene to open it.
2. Click play.
3. Using the UI, enter an APIKey in the input field to the top left.
4. Open the **Samples** drop-down and click **Building Filter** to open the level.
5. Use the UI to toggle visibility of different levels, construction phases, disciplines, and categories.

## How to use the sample (BuildingFilter Scene)
1. **Open the BuildingFilter Level:**
   - Navigate to the **BuildingFilter** level in your project and open it.
2. **Set the API Key:**
   - In the outliner, locate and select the **ArcGISMapActor**.
   - In the **Details** panel, find the field for the API key and enter your API key.
3. **Start the Simulation:**
   - Click the play button to start the simulation.
4. **Interact with the UI:**
   - **Service URL:**
     - There is a space in the UI where you can add a service URL for a building scene layer. Enter the URL to load a specific building scene layer. You can also use a local file path for the URL.
   - **Building Scene Levels:**
     - Use the provided controls to adjust the building scene levels. This allows you to focus on specific levels of the building.
   - **Construction Phases:**
     - Use the slider to adjust the construction phases of the building scene layer. Slide it to view different stages of the construction process.
   - **Disciplines and Categories:**
     - The UI provides a list of disciplines and categories. You can:
       - Set the visibility of specific disciplines or categories by toggling them on or off.
       - Enable or disable all disciplines or categories at once using the provided options.

## How it works
1. Create an ArcGISMap actor with a building scene layer.
2. Create a default building filter to apply different where clauses.
3. Create an actor with a C++ script to filter different criteria.
4. The [`GetSubLayers`](https://developers.arcgis.com/unreal-engine/api-reference/gameengine/layers/buildingscene/arcgisbuildingscenesublayer/#sublayers) function is used to fetch a collection of the building scene layer's sublayers.
5. The [`GetSolidFilterDefinition`](https://developers.arcgis.com/unreal-engine/api-reference/gameengine/layers/buildingscene/arcgisbuildingattributefilter/#solidfilterdefinition) function is used to get the filter definition of the default filter.
6. The [`SetWhereClause`](https://developers.arcgis.com/unreal-engine/api-reference/gameengine/layers/buildingscene/arcgissolidbuildingfilterdefinition/#whereclause) function is used to set a dynamically created where clause based on filter criteria.
7. The [`SetActiveBuildingAttributeFilter`](https://developers.arcgis.com/unreal-engine/api-reference/gameengine/layers/arcgisbuildingscenelayer/#activebuildingattributefilter) function is used to set the default filter with the new where clause to active.

## Tags
building scene layer, filter, disciplines, categories, construction phases

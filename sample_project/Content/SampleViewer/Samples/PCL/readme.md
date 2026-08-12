# Visualize and filter a point cloud layer

Load a point cloud scene layer and explore different renderers, display settings, and attribute filters.

## How to use the sample (SampleViewer)

1. Open the **SampleViewer** level if it is not already open.
2. Click Play.
3. Enter an API key in the input field at the top left.
4. Open the **Samples** drop-down and select **PCL**.
5. Use the sample UI to customize, visualize, and filter the point cloud layer.

## How to use the sample (PointCloudLayer level)

1. Open the **PointCloudLayer** level.
2. Select the **ArcGISMapActor** in the Outliner.
3. Enter an API key under **Authentication** in the Details panel.
4. Click Play. The default point cloud scene layer loads automatically, and the camera zooms to its extent.
5. Use the tabs in the UI:
   - **Customize** changes the point size and point density. Enter another point cloud scene service URL and click **Load** to replace the current layer.
   - **Visualize** switches between RGB, classification, elevation, and intensity renderers. Renderer options are available only when the loaded layer contains the required attributes. Color modulation can add intensity-based shading when an intensity attribute is available.
   - **Filter** includes or excludes points by classification and return type. Use **Reset** to restore all filter selections.
6. Use the collapse control to hide or restore the main UI and legend. The instruction panel remains available independently.

## How it works

1. The `APCLController` finds the `ArcGISMapActor`, gets its map component, and creates the sample UI.
2. An [`ArcGISPointCloudLayer`](https://developers.arcgis.com/unreal-engine/api-reference/gameengine/layers/arcgispointcloudlayer/) is created from the scene service URL and added to the map. After the asynchronous load completes, the sample zooms to the layer extent.
3. The controller reads the point cloud attributes and enables only the renderer and filter controls supported by the loaded layer.
4. The sample applies one of four point cloud renderers:
   - RGB uses the layer's color values.
   - Classification uses unique colors for standard classification codes.
   - Elevation uses a color gradient based on height values.
   - Intensity uses a grayscale gradient based on intensity values.
5. A fixed-size algorithm controls point size, while the points-per-inch setting controls point density. Optional color modulation shades the selected renderer with intensity values.
6. Classification selections create an `ArcGISPointCloudValueFilter`, and return selections create an `ArcGISPointCloudReturnFilter`. The active filters are combined and applied to the layer.
7. The UI builds a matching legend for the selected renderer and updates it when the renderer or layer changes.

## About the data

This sample loads the [BARNEGAT BAY LiDAR UTM point cloud scene service](https://tiles.arcgis.com/tiles/V6ZHFr6zdgNZuVG0/arcgis/rest/services/BARNEGAT_BAY_LiDAR_UTM/SceneServer).

## Tags

point cloud layer, LiDAR, renderer, classification, elevation, intensity, filter

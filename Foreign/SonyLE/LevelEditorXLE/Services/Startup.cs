﻿// Copyright 2015 XLGAMES Inc.
//
// Distributed under the MIT License (See
// accompanying file "LICENSE" or the website
// http://www.opensource.org/licenses/mit-license.php)

using System.ComponentModel;
using System.ComponentModel.Composition.Hosting;

using Sce.Atf;
using Sce.Atf.Dom;

using LevelEditorCore;
using LevelEditorXLE.Terrain;
using LevelEditorXLE.Placements;
using LevelEditorXLE.Game;
using LevelEditorXLE.Environment;

using PropertyDescriptor = Sce.Atf.Dom.PropertyDescriptor;

namespace LevelEditorXLE
{
    public static class Startup
    {
        public static void InitializeAdapters()
        {
            Schema.placementsCellReferenceType.Type.Define(new ExtensionInfo<PlacementsCellRef>());
            Schema.placementsFolderType.Type.Define(new ExtensionInfo<PlacementsFolder>());
            Schema.placementsDocumentType.Type.Define(new ExtensionInfo<XLEPlacementDocument>());
            Schema.placementObjectType.Type.Define(new ExtensionInfo<XLEPlacementObject>());
            Schema.terrainType.Type.Define(new ExtensionInfo<XLETerrainGob>());
            Schema.envSettingsFolderType.Type.Define(new ExtensionInfo<XLEEnvSettingsFolder>());
            Schema.envSettingsType.Type.Define(new ExtensionInfo<XLEEnvSettings>());
            Schema.xleGameType.Type.Define(new ExtensionInfo<GameExtensions>());
        }

        public static TypeCatalog CreateTypeCatalog()
        {
            return new TypeCatalog(
                typeof(XLELayer.TerrainManipulator),
                typeof(XLELayer.PlacementManipulator),
                typeof(Manipulators.XLEManipCtrlWin),
                typeof(Materials.XLEMaterialInspector),
                typeof(XLELayer.XLEAssetService),
                typeof(XLECamera),
                typeof(XLELayer.MaterialSchemaLoader),
                typeof(Placements.ResourceConverter)
                );
        }
    }
}
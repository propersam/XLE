﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

using Sce.Atf;
using Sce.Atf.Adaptation;
using Sce.Atf.Applications;
using Sce.Atf.Dom;

using LevelEditorCore;

namespace LevelEditor.DomNodeAdapters
{
    public class XLEEnvSettings : DomNodeAdapter, IHierarchical, IListable
    {
        public bool CanAddChild(object child)
        {
            var domNode = child as DomNode;
            if (domNode != null)
            {
                foreach (var type in domNode.Type.Lineage)
                {
                    if (type == Schema.envObjectType.Type) return true;
                    if (type == Schema.envMiscType.Type) return true;
                    if (type == Schema.ambientSettingsType.Type) return true;
                    if (type == Schema.toneMapSettingsType.Type) return true;
                }
            }
            return false;
        }

        public bool AddChild(object child)
        {
            var domNode = child.As<DomNode>();
            if (domNode != null)
            {
                foreach (var type in domNode.Type.Lineage)
                {
                    if (type == Schema.envObjectType.Type)
                    {
                        GetChildList<DomNode>(Schema.envSettingsType.objectsChild).Add(domNode);
                        return true;
                    }
                    if (type == Schema.envMiscType.Type)
                    {
                        GetChildList<DomNode>(Schema.envSettingsType.settingsChild).Add(domNode);
                        return true;
                    }
                    if (type == Schema.ambientSettingsType.Type)
                    {
                        SetChild(Schema.envSettingsType.ambientChild, domNode);
                        return true;
                    }
                    if (type == Schema.toneMapSettingsType.Type)
                    {
                        SetChild(Schema.envSettingsType.tonemapChild, domNode);
                        return true;
                    }
                }
            }
            return false;
        }

        public void GetInfo(ItemInfo info)
        {
            info.ImageIndex = Util.GetTypeImageIndex(DomNode.Type, info.GetImageList());
            info.Label = "Settings: " + Name;
        }

        public string Name
        {
            get { return GetAttribute<string>(Schema.envSettingsType.nameAttribute); }
        }
    }

    public class XLEEnvSettingsFolder : DomNodeAdapter, IHierarchical, IListable
    {
        public void GetInfo(ItemInfo info)
        {
            info.ImageIndex = Util.GetTypeImageIndex(DomNode.Type, info.GetImageList());
            info.Label = "EnvSettingsFolder";
        }

        public bool CanAddChild(object child)
        {
            return child.Is<XLEEnvSettings>();
        }

        public bool AddChild(object child)
        {
            var settings = child.As<XLEEnvSettings>();
            if (settings != null) {
                Settings.Add(settings);
                return true;
            }
            return false;
        }

        public System.Collections.Generic.ICollection<XLEEnvSettings> Settings
        {
            get
            {
                return GetChildList<XLEEnvSettings>(Schema.envSettingsFolderType.settingsChild);
            }
        }
    }

}
using System.IO;
using System;
using System.Collections.Generic;
using Newtonsoft.Json;

namespace JoyAssetBuilder
{
    public class MaterialBuilder
    {
        [Serializable]
        class MaterialObject
        {
            public string name;
            public string diffuse;
        }

        [Serializable]
        class MaterialListObject
        {
            public string type;
            public List<MaterialObject> materials;
        }

        public static bool BuildMaterial(string materialPath, out string resultMessage)
        {
            MaterialListObject listObject = new MaterialListObject();
            listObject.type = "standard_material_list";
            listObject.materials = new List<MaterialObject>();

            string[] mtlParams = File.ReadAllText(materialPath).Split(new[] { '\r', '\n' });
            MaterialObject currentMaterialObject = null;
            for (int i = 0; i < mtlParams.Length; i++)
            {
                string[] split = mtlParams[i].Split(' ');
                if (split.Length < 2) continue;

                string firstEntry = split[0];
                string secondEntry = split[1];
                if (firstEntry == "newmtl")
                {
                    MaterialObject mat = new MaterialObject();
                    mat.name = secondEntry;
                    mat.diffuse = "";
                    listObject.materials.Add(mat);
                    currentMaterialObject = mat;
                }

                if (firstEntry == "map_Kd" && currentMaterialObject != null)
                {
                    currentMaterialObject.diffuse = secondEntry;
                }
            }

            string output = JsonConvert.SerializeObject(listObject, Formatting.Indented);
            File.WriteAllText(materialPath + ".json", output);

            resultMessage = Path.GetFileName(materialPath) + ": OK" + Environment.NewLine;
            return true;
        }
    }
}
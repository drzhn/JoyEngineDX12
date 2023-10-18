using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Newtonsoft.Json;

namespace JoyAssetBuilder
{
    public class DatabaseBuilder
    {
        [Serializable]
        class DatabaseEntry
        {
            public string guid;
            public string path;
        }

        private readonly string m_dataPath;
        private readonly string m_databasePath;
        private const string m_databaseFilename = "data.db";

        private HashSet<string> m_allowedExtensions = new HashSet<string>()
        {
            ".obj",

            ".png",
            ".jpg",
            ".jpeg",
            ".hdr",
            ".tga",
            ".dds",

            ".json",
            ".hlsl"
        };

        public DatabaseBuilder(string dataPath)
        {
            m_dataPath = dataPath;
            m_databasePath = Path.Combine(m_dataPath, m_databaseFilename);
        }
    }
}
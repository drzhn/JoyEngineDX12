﻿using System;
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

        [Serializable]
        class DatabaseStorage
        {
            public string type;
            public List<DatabaseEntry> database;
        }

        private readonly string m_dataPath;
        private readonly string m_databasePath;
        private const string m_databaseFilename = "data.db";
        private readonly DatabaseStorage m_databaseStorage;

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
            m_databaseStorage = JsonConvert.DeserializeObject<DatabaseStorage>(File.ReadAllText(m_databasePath));
        }

        public void RebuildDatabase()
        {
            Dictionary<string, string> databaseDictionary = new Dictionary<string, string>();
            foreach (var entry in m_databaseStorage.database)
            {
                databaseDictionary.Add(entry.path, entry.guid);
            }

            List<string> keysToDelete =
                databaseDictionary.Keys.Where(x => !File.Exists(Path.Combine(m_dataPath, x))).ToList();
            foreach (string key in keysToDelete)
            {
                databaseDictionary.Remove(key);
            }

            foreach (var file in Directory.GetFiles(m_dataPath, "*.*", SearchOption.AllDirectories))
            {
                if (!m_allowedExtensions.Contains(Path.GetExtension(file))) continue;
                string relativePath = file.Substring(m_dataPath.Length + 1).Replace('\\', '/');
                if (!databaseDictionary.ContainsKey(relativePath))
                {
                    databaseDictionary.Add(relativePath, Guid.NewGuid().ToString());
                }
            }

            m_databaseStorage.database = new List<DatabaseEntry>();
            foreach (var key in databaseDictionary.Keys)
            {
                DatabaseEntry entry = new DatabaseEntry();
                entry.guid = databaseDictionary[key];
                entry.path = key;
                m_databaseStorage.database.Add(entry);
            }

            File.WriteAllText(m_databasePath, JsonConvert.SerializeObject(m_databaseStorage, Formatting.Indented));
        }
    }
}
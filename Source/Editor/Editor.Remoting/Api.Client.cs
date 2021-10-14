using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using Newtonsoft.Json;

namespace Editor.Remoting
{
    static class ClientSettings
	{
        public static void UpdateJsonSerializerSettings(JsonSerializerSettings settings)
		{
        }
    }

    public partial class MetaObject
    {
        [JsonProperty]
        public string _class { get; set; }
    }

    public partial class RTTITraits
    {
        public RTTITraitsScalar AsScalar { get => this as RTTITraitsScalar; }
        public RTTITraitsTuple AsTuple { get => this as RTTITraitsTuple; }
        public RTTITraitsList AsList { get => this as RTTITraitsList; }
        public RTTITraitsDico AsDico { get => this as RTTITraitsDico; }
    }

    public partial class RTTIEndpointClient
    {
        partial void UpdateJsonSerializerSettings(JsonSerializerSettings settings)
        {
            ClientSettings.UpdateJsonSerializerSettings(settings);
        }
    }

    public partial class ProcessEndpointClient
    {
        partial void UpdateJsonSerializerSettings(JsonSerializerSettings settings)
        {
            ClientSettings.UpdateJsonSerializerSettings(settings);
        }
    }
}

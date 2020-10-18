// Configuration.h

#ifndef _CONFIGURATION_h
#define _CONFIGURATION_h

#define MAX_CONFIG_SIZE		8	// number of config entries

#define FIELD_SSID			"SSID"
#define FIELD_PASSWORD	"Password"

struct Property
{
	const char* Id = NULL;
	const char* Name = NULL;
	size_t Len = 0;
};

class Dictionary;
class ConfigurationClass
{
	protected:
		Property _Schema[MAX_CONFIG_SIZE];
		size_t _SchemaSize = 0;
		Dictionary* _pConfig = NULL;

	public:
		ConfigurationClass();
		~ConfigurationClass();

		bool AddSchemaField(const char* id, const char* name, size_t len);
		bool LoadFromFile();
		bool IsConfigComplete();
		bool ReConfig();
		bool SaveToFile();

		void PrintConfig();

		String Get(const char* varname);
};

extern ConfigurationClass GlobalConfig;

#endif

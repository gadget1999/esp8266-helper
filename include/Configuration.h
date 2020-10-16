// Configuration.h

#ifndef _CONFIGURATION_h
#define _CONFIGURATION_h

#define MAX_CONFIG_SIZE		32

struct Property
{
	const char* Id = NULL;
	const char* Name = NULL;
	int Len = 0;
};

class Dictionary;

class ConfigurationClass
{
	protected:
		Property _Schema[MAX_CONFIG_SIZE];
		size_t _SchemaSize = 0;
		Dictionary* _pConfig = NULL;

		bool LoadConfig();
	
	public:
		ConfigurationClass();
		~ConfigurationClass();

		bool AddSchemaField(const char* id, const char* name, int len);
		bool Setup();
		bool Loop();

		bool ReConfig();
		bool Save();

		String Get(const char* varname);
};

extern ConfigurationClass GlobalConfig;

#endif

#include "lilv/lilv.h"
#include "../src/lilv_internal.h"

#define PLUGIN_URI "http://example.org/missing-port-name"

#define TEST_ASSERT(check) do {\
	if (!(check)) {\
		fprintf(stderr, "%s:%d: failed test: %s\n", __FILE__, __LINE__, #check);\
		return 1;\
	}\
} while (0)

int
main(int argc, char** argv)
{
	if (argc != 2) {
		fprintf(stderr, "USAGE: %s BUNDLE\n", argv[0]);
		return 1;
	}

	const char* bundle_path = argv[1];
	LilvWorld*  world       = lilv_world_new();

	// Load test plugin bundle
	uint8_t*  abs_bundle = (uint8_t*)lilv_path_absolute(bundle_path);
	SerdNode  bundle     = serd_node_new_file_uri(abs_bundle, 0, 0, true);
	LilvNode* bundle_uri = lilv_new_uri(world, (const char*)bundle.buf);
	lilv_world_load_bundle(world, bundle_uri);
	free(abs_bundle);
	serd_node_free(&bundle);

	LilvNode*          plugin_uri = lilv_new_uri(world, PLUGIN_URI);
	const LilvPlugins* plugins    = lilv_world_get_all_plugins(world);
	const LilvPlugin*  plugin     = lilv_plugins_get_by_uri(plugins, plugin_uri);
	TEST_ASSERT(plugin);

	const LilvPort* port = lilv_plugin_get_port_by_index(plugin, 0);
	TEST_ASSERT(port);
	LilvNode* name = lilv_port_get_name(plugin, port);
	TEST_ASSERT(!name);

	lilv_world_free(world);

	return 0;
}


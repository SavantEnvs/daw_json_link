#include <cstdint>

#include "FuzzedDataProvider.h"
#include "daw/json/daw_json_link.h"

struct MyClass1 {
	std::string name;
	std::chrono::time_point<std::chrono::system_clock,
	                        std::chrono::milliseconds>
	  timestamp;
};

bool operator==( MyClass1 const &lhs, MyClass1 const &rhs ) {
	return std::tie( lhs.name, lhs.timestamp ) ==
	       std::tie( rhs.name, rhs.timestamp );
}
namespace daw::json {
	template<>
	struct json_data_contract<MyClass1> {
#if defined( DAW_JSON_CNTTP_JSON_NAME )
		using type = json_member_list<json_string<"name">, json_date<"timestamp">>;
#else
		static constexpr char const name[] = "name";
		static constexpr char const timestamp[] = "timestamp";
		using type = json_member_list<json_string<name>, json_date<timestamp>>;
#endif
		static inline auto
		to_json_data( MyClass1 const &value ) {
			return std::forward_as_tuple( value.name, value.timestamp );
		}
	};
} // namespace daw::json


extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	FuzzedDataProvider fdp(data, size);

	try {
		auto fuzz = daw::json::from_json<MyClass1>( fdp.ConsumeRemainingBytesAsString( ) );
		(void)fuzz;
	} catch (daw::json::json_exception const &e) {
		// A rejected parse is EXPECTED for most fuzz inputs — return 0, not -1.
		// libFuzzer treats a -1 return as "do not add this input to the corpus",
		// so returning -1 on every throwing input keeps the corpus from ever
		// growing: from a cold/empty corpus (Mayhem's first run) the fuzzer can
		// never retain a coverage-increasing input and stays at 0 edges forever.
		(void)e;
	}
  return 0;
}
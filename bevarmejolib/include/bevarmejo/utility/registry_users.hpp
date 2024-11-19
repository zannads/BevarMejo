#pragma once
#ifndef BEVARMEJOLIB__UTILITY__REGISTRY_USERS_HPP
#define BEVARMEJOLIB__UTILITY__REGISTRY_USERS_HPPS

// Write here the forward definition of the classes that will be friends of the Registry.
#ifndef FORWARD_DEFINITIONS_REGISTRY_FRIEND_CLASSES
#define FORWARD_DEFINITIONS_REGISTRY_FRIEND_CLASSES \
namespace bevarmejo::wds { \
    class WaterDistributionSystem; \
} // namespace bevarmejo::wds \
// Add here other forward definitions and divide them with a backslash.
#endif // FORWARD_DEFINITIONS_REGISTRY_FRIEND_CLASSES

// Write here the friend relationship that will be substituted in the registry definition.
#ifndef FRIEND_RELATIONSHIPS_FOR_REGISTRY
#define FRIEND_RELATIONSHIPS_FOR_REGISTRY \
friend class bevarmejo::wds::WaterDistributionSystem; \
// Add here other friend relationships and divide them with a backslash.
#endif // FRIEND_RELATIONSHIPS_FOR_REGISTRY

#endif // BEVARMEJOLIB__UTILITY__REGISTRY_USERS_HPP

#ifndef IVW_PROCESSORFACTORY_H
#define IVW_PROCESSORFACTORY_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/util/inviwofactorybase.h>
#include <inviwo/core/util/singleton.h>

namespace inviwo {

class IVW_CORE_API PropertyFactory : public Factory,
                                     public Singleton<PropertyFactory> {

public:
    PropertyFactory();
    ~PropertyFactory();

    virtual void initialize();
    virtual void deinitialize();

    void registerFactoryObject(Property* property);
    virtual IvwSerializable* create(std::string className) const;

private:
    mutable std::map<std::string, Property*> processorClassMap_;
};

} // namespace

#endif // IVW_PROCESSORFACTORY_H

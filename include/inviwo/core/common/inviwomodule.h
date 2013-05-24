#ifndef IVW_INVIWOMODULE_H
#define IVW_INVIWOMODULE_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorfactoryobject.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datawriter.h>
#include <inviwo/core/datastructures/representationconverter.h>
#include <inviwo/core/metadata/metadata.h>
#include <inviwo/core/util/resourceinfo.h>
#include <inviwo/core/util/settings.h>

namespace inviwo {

class IVW_CORE_API InviwoModule {

public:
    InviwoModule();
    virtual ~InviwoModule();

    std::string getIdentifier() const;

    bool isInitialized() const;

    const std::vector<ProcessorFactoryObject*>& getProcessors() const;
    const std::vector<std::pair<std::string, ProcessorWidget*>>& getProcessorWidgets() const;
    const std::vector<Property*>& getProperties() const;
    const std::vector<DataReader*>& getDataReaders() const;
    const std::vector<DataWriter*>& getDataWriters() const;
    const std::vector<MetaData*>& getMetaData() const;
    const std::vector<RepresentationConverter*>& getRepresentationConverters() const;
    std::vector<ResourceInfo*>& getResourceInfos();
    
    std::string getDescription() const;
    void setDescription(const std::string& description) const;
    void setGlobalSettings(Settings* settings);

    virtual void initialize();
    virtual void deinitialize();

protected:

    Settings* getSettings() { return applicationSettings_; }

    void setIdentifier(const std::string& identifier);

    void addProcessor(ProcessorFactoryObject* processor);
    void addProcessorWidget(std::string processorClassName, ProcessorWidget* processorWidget);
    void addProperty(Property* property);
    void addDataReader(DataReader* reader);
    void addDataWriter(DataWriter* writer);
    void addMetaData(MetaData* meta);
    void addRepresentationConverter(RepresentationConverter* representationConverter);
    void addResourceInfo(ResourceInfo* info);

    void setXMLFileName(const std::string& xmlDocuFileName);

    virtual void setupModuleSettings(){};

    std::string getPath(const std::string& suffix = "") const;

private:
    std::string identifier_;

    bool initialized_;

    std::vector<ProcessorFactoryObject*> processors_;
    std::vector<std::pair<std::string, ProcessorWidget*>> processorWidgets_;
    std::vector<Property*> properties_;
    std::vector<DataReader*> dataReaders_;
    std::vector<DataWriter*> dataWriters_;
    std::vector<MetaData*> metadata_;
    std::vector<RepresentationConverter*> representationConverters_;
    std::vector<ResourceInfo*> resourceInfos_;

    Settings* applicationSettings_;

    std::string xmlDocuFileName_;
};

#define registerProcessor(T) { addProcessor(new ProcessorFactoryObjectTemplate<T>(T::CLASS_NAME, T::CATEGORY, T::CODE_STATE)); }

} // namespace

#endif // IVW_INVIWOMODULE_H

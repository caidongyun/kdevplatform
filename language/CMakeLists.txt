add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9505 )

add_subdirectory(highlighting/tests)
add_subdirectory(duchain/tests)
add_subdirectory(backgroundparser/tests)
add_subdirectory(codegen/tests)

set(kdevplatformlanguage_LIB_SRCS
    editor/persistentmovingrangeprivate.cpp
    editor/persistentmovingrange.cpp
    editor/modificationrevisionset.cpp
    editor/modificationrevision.cpp
    backgroundparser/backgroundparser.cpp
    backgroundparser/parsejob.cpp
    backgroundparser/parserdependencypolicy.cpp
    backgroundparser/documentchangetracker.cpp
    backgroundparser/parseprojectjob.cpp
    backgroundparser/urlparselock.cpp

    duchain/referencecounting.cpp
    duchain/specializationstore.cpp
    duchain/codemodel.cpp
    duchain/duchain.cpp
    duchain/waitforupdate.cpp
    duchain/duchainpointer.cpp
    duchain/ducontext.cpp
    duchain/indexedducontext.cpp
    duchain/indexedtopducontext.cpp
    duchain/localindexedducontext.cpp
    duchain/indexeddeclaration.cpp
    duchain/localindexeddeclaration.cpp
    duchain/topducontext.cpp
    duchain/topducontextdynamicdata.cpp
    duchain/functiondefinition.cpp
    duchain/declaration.cpp
    duchain/classmemberdeclaration.cpp
    duchain/classfunctiondeclaration.cpp
    duchain/classdeclaration.cpp
    duchain/use.cpp
    duchain/forwarddeclaration.cpp
    duchain/duchainbase.cpp
    duchain/duchainlock.cpp
    duchain/identifier.cpp
    duchain/parsingenvironment.cpp
    duchain/abstractfunctiondeclaration.cpp
    duchain/functiondeclaration.cpp
    duchain/stringhelpers.cpp
    duchain/namespacealiasdeclaration.cpp
    duchain/aliasdeclaration.cpp
    duchain/dumpdotgraph.cpp
    duchain/duchainobserver.cpp
    duchain/duchainutils.cpp
    duchain/declarationid.cpp
    duchain/definitions.cpp
    duchain/uses.cpp
    duchain/importers.cpp
    duchain/dumpchain.cpp
    duchain/indexedstring.cpp
    duchain/duchainregister.cpp
    duchain/persistentsymboltable.cpp
    duchain/instantiationinformation.cpp

    duchain/types/typesystem.cpp
    duchain/types/typeregister.cpp
    duchain/types/identifiedtype.cpp
    duchain/types/abstracttype.cpp
    duchain/types/integraltype.cpp
    duchain/types/functiontype.cpp
    duchain/types/structuretype.cpp
    duchain/types/pointertype.cpp
    duchain/types/referencetype.cpp
    duchain/types/delayedtype.cpp
    duchain/types/arraytype.cpp
    duchain/types/indexedtype.cpp
    duchain/types/enumerationtype.cpp
    duchain/types/constantintegraltype.cpp
    duchain/types/enumeratortype.cpp
    duchain/types/typeutils.cpp
    duchain/types/typealiastype.cpp
    duchain/types/unsuretype.cpp

    duchain/repositories/itemrepository.cpp
    duchain/repositories/typerepository.cpp

    duchain/navigation/problemnavigationcontext.cpp
    duchain/navigation/abstractnavigationwidget.cpp
    duchain/navigation/abstractnavigationcontext.cpp
    duchain/navigation/usesnavigationcontext.cpp
    duchain/navigation/abstractdeclarationnavigationcontext.cpp
    duchain/navigation/abstractincludenavigationcontext.cpp
    duchain/navigation/useswidget.cpp
    duchain/navigation/usescollector.cpp

    interfaces/iastcontainer.cpp
    interfaces/ilanguagesupport.cpp
    interfaces/quickopendataprovider.cpp
    interfaces/iproblem.cpp
    interfaces/iquickopen.cpp
    interfaces/editorcontext.cpp
    interfaces/codecontext.cpp
    interfaces/icreateclasshelper.cpp
    interfaces/icontextbrowser.cpp

    codecompletion/codecompletion.cpp
    codecompletion/codecompletionworker.cpp
    codecompletion/codecompletionmodel.cpp
    codecompletion/codecompletionitem.cpp
    codecompletion/codecompletioncontext.cpp
    codecompletion/codecompletionitemgrouper.cpp
    codecompletion/codecompletionhelper.cpp
    codecompletion/normaldeclarationcompletionitem.cpp

    codegen/applychangeswidget.cpp
    codegen/coderepresentation.cpp
    codegen/documentchangeset.cpp
    codegen/duchainchangeset.cpp
    codegen/komparesupport.cpp
    codegen/utilities.cpp
    codegen/templatesmodel.cpp
    codegen/templateclassgenerator.cpp
    codegen/codedescription.cpp
    codegen/sourcefiletemplate.cpp
    codegen/templaterenderer.cpp
    codegen/templateengine.cpp
    codegen/archivetemplateloader.cpp

    util/setrepository.cpp
    util/includeitem.cpp
    util/navigationtooltip.cpp

    highlighting/colorcache.cpp
    highlighting/configurablecolors.cpp
    highlighting/codehighlighting.cpp

    checks/dataaccessrepository.cpp checks/dataaccess.cpp
    checks/controlflowgraph.cpp checks/controlflownode.cpp
)

kde4_add_library(kdevplatformlanguage SHARED ${kdevplatformlanguage_LIB_SRCS})
target_link_libraries(kdevplatformlanguage
        ${KDE4_KPARTS_LIBS}
        ${KDE4_KTEXTEDITOR_LIBS}
        ${KDE4_THREADWEAVER_LIBRARIES}
        ${KDE4_KNEWSTUFF3_LIBS}
        kdevplatforminterfaces
        kdevplatformutil
        ${Grantlee_CORE_LIBRARIES})
# Need to add kdevplatforminterfaces and kdevplatformutil when those are exported targets
target_link_libraries(kdevplatformlanguage LINK_INTERFACE_LIBRARIES
        kdevplatforminterfaces)
set_target_properties(kdevplatformlanguage PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${KDEVPLATFORM_LIB_SOVERSION})
install(TARGETS kdevplatformlanguage EXPORT KDevPlatformTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install(FILES
    languageexport.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/language COMPONENT Devel
)

install(FILES
    interfaces/ilanguagesupport.h
    interfaces/icodehighlighting.h
    interfaces/quickopendataprovider.h
    interfaces/quickopenfilter.h
    interfaces/iquickopen.h
    interfaces/iproblem.h
    interfaces/codecontext.h
    interfaces/editorcontext.h
    interfaces/iastcontainer.h
    interfaces/icreateclasshelper.h
    interfaces/icontextbrowser.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/language/interfaces COMPONENT Devel
)

install(FILES
    editor/persistentmovingrange.h
    editor/documentrange.h
    editor/documentcursor.h
    editor/simplecursor.h
    editor/simplerange.h
    editor/cursorinrevision.h
    editor/rangeinrevision.h
    editor/modificationrevision.h
    editor/modificationrevisionset.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/language/editor COMPONENT Devel
)

install(FILES
    backgroundparser/backgroundparser.h
    backgroundparser/parsejob.h
    backgroundparser/parseprojectjob.h
    backgroundparser/urlparselock.h
    backgroundparser/documentchangetracker.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/language/backgroundparser COMPONENT Devel
)

install(FILES
    util/navigationtooltip.h
    util/setrepository.h
    util/basicsetrepository.h
    util/includeitem.h
    util/debuglanguageparserhelper.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/language/util COMPONENT Devel
)

install(FILES
    duchain/referencecounting.h
    duchain/parsingenvironment.h
    duchain/duchain.h
    duchain/codemodel.h
    duchain/ducontext.h
    duchain/ducontextdata.h
    duchain/topducontext.h
    duchain/topducontextutils.h
    duchain/topducontextdata.h
    duchain/declaration.h
    duchain/declarationdata.h
    duchain/classmemberdeclaration.h
    duchain/classmemberdeclarationdata.h
    duchain/classfunctiondeclaration.h
    duchain/classdeclaration.h
    duchain/functiondefinition.h
    duchain/use.h
    duchain/forwarddeclaration.h
    duchain/duchainbase.h
    duchain/duchainpointer.h
    duchain/duchainobserver.h
    duchain/duchainlock.h
    duchain/identifier.h
    duchain/abstractfunctiondeclaration.h
    duchain/functiondeclaration.h
    duchain/stringhelpers.h
    duchain/safetycounter.h
    duchain/namespacealiasdeclaration.h
    duchain/aliasdeclaration.h
    duchain/dumpdotgraph.h
    duchain/duchainutils.h
    duchain/dumpchain.h
    duchain/indexedstring.h
    duchain/declarationid.h
    duchain/appendedlist.h
    duchain/duchainregister.h
    duchain/persistentsymboltable.h
    duchain/instantiationinformation.h
    duchain/specializationstore.h
    duchain/persistentsetmap.h
    duchain/indexedducontext.h
    duchain/indexedtopducontext.h
    duchain/localindexedducontext.h
    duchain/indexeddeclaration.h
    duchain/localindexeddeclaration.h
    duchain/definitions.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/language/duchain COMPONENT Devel
)

install(FILES
    duchain/types/unsuretype.h
    duchain/types/identifiedtype.h
    duchain/types/typesystem.h
    duchain/types/typeregister.h
    duchain/types/typepointer.h
    duchain/types/typesystemdata.h
    duchain/types/abstracttype.h
    duchain/types/integraltype.h
    duchain/types/functiontype.h
    duchain/types/structuretype.h
    duchain/types/pointertype.h
    duchain/types/referencetype.h
    duchain/types/delayedtype.h
    duchain/types/arraytype.h
    duchain/types/indexedtype.h
    duchain/types/enumerationtype.h
    duchain/types/constantintegraltype.h
    duchain/types/enumeratortype.h
    duchain/types/alltypes.h
    duchain/types/typeutils.h
    duchain/types/typealiastype.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/language/duchain/types COMPONENT Devel
)

install(FILES
    duchain/builders/abstractcontextbuilder.h
    duchain/builders/abstractdeclarationbuilder.h
    duchain/builders/abstracttypebuilder.h
    duchain/builders/abstractusebuilder.h
    duchain/builders/abstractexpressionvisitor.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/language/duchain/builders COMPONENT Devel
)

install(FILES
    duchain/repositories/itemrepository.h
    duchain/repositories/typerepository.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/language/duchain/repositories COMPONENT Devel
)

install(FILES
    codecompletion/codecompletion.h
    codecompletion/codecompletionworker.h
    codecompletion/codecompletionmodel.h
    codecompletion/codecompletionitem.h
    codecompletion/codecompletioncontext.h
    codecompletion/codecompletionitemgrouper.h
    codecompletion/codecompletionhelper.h
    codecompletion/normaldeclarationcompletionitem.h
    codecompletion/abstractincludefilecompletionitem.h
    codecompletion/codecompletiontesthelper.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/language/codecompletion COMPONENT Devel
)

install(FILES
    codegen/applychangeswidget.h
    codegen/astchangeset.h
    codegen/duchainchangeset.h
    codegen/documentchangeset.h
    codegen/coderepresentation.h
    codegen/utilities.h
    codegen/templatesmodel.h
    codegen/templaterenderer.h
    codegen/templateengine.h
    codegen/sourcefiletemplate.h
    codegen/templateclassgenerator.h
    codegen/codedescription.h
    codegen/sourcefiletemplate.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/language/codegen COMPONENT Devel
)

install(FILES
    duchain/navigation/usesnavigationcontext.h
    duchain/navigation/abstractnavigationcontext.h
    duchain/navigation/abstractdeclarationnavigationcontext.h
    duchain/navigation/abstractincludenavigationcontext.h
    duchain/navigation/abstractnavigationwidget.h
    duchain/navigation/navigationaction.h
    duchain/navigation/useswidget.h
    duchain/navigation/usescollector.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/language/duchain/navigation COMPONENT Devel
)

install(FILES
    highlighting/codehighlighting.h
    highlighting/colorcache.h
    highlighting/configurablecolors.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/language/highlighting COMPONENT Devel
)

install(FILES
    checks/dataaccess.h
    checks/dataaccessrepository.h
    checks/controlflowgraph.h
    checks/controlflownode.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/language/checks COMPONENT Devel
)



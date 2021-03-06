#SET_DIRECTORY_PROPERTIES(PROPERTIES CLEAN_NO_CUSTOM true)

OPENCOG_ADD_ATOM_TYPES(atom_types.script 
                      atom_types.h
                      atom_types.definitions
                      atom_types.inheritance
                      core_types.scm)
ADD_CUSTOM_TARGET(opencog_atom_types DEPENDS atom_types.h)

# The atom_types.h file is written to the build directory
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

ADD_LIBRARY (atomspace SHARED
    atom_types.h
    Atom.cc
    AtomSpace.cc
    AtomSpaceAsync.cc
    AtomSpaceImpl.cc
    AtomSpaceInit.cc
    AtomTable.cc
    AttentionValue.cc
    AttentionBank.cc
    ClassServer.cc
    CompositeTruthValue.cc
    CountTruthValue.cc
    FixedIntegerIndex.cc
    Handle.cc
    HandleSeqIndex.cc
    HandleSetIndex.cc
    ImportanceIndex.cc
    IncomingIndex.cc
    IndefiniteTruthValue.cc
    Intersect.cc
    Link.cc
    LinkIndex.cc
    NameIndex.cc
    Node.cc
    NodeIndex.cc
    NullTruthValue.cc
    PredicateIndex.cc
    SimpleTruthValue.cc
    StatisticsMonitor.cc
    StringIndex.cc
    TargetTypeIndex.cc
    TLB.cc
    Trail.cc
    TruthValue.cc
    TypeIndex.cc
    VersionHandle.cc
)

# Without this, parallel make will race and crap up the generated files.
ADD_DEPENDENCIES(atomspace opencog_atom_types)

SET(ATOMSPACE_LINK_LIBS
	cogutil
	${GSL_LIBRARIES}
	${Boost_SIGNALS_LIBRARY}
	${Boost_THREAD_LIBRARY}
)

IF (HAVE_ZMQ)
	SET(ATOMSPACE_LINK_LIBS ${ATOMSPACE_LINK_LIBS} zmq)
ENDIF (HAVE_ZMQ)

TARGET_LINK_LIBRARIES(atomspace
	${ATOMSPACE_LINK_LIBS}
)

IF (CYGWIN)
	INSTALL (TARGETS atomspace
		DESTINATION "lib${LIB_DIR_SUFFIX}/opencog"
	)
ELSE (CYGWIN)
	INSTALL (TARGETS atomspace
		LIBRARY DESTINATION "lib${LIB_DIR_SUFFIX}/opencog"
	)
ENDIF (CYGWIN)

INSTALL (FILES
    ASRequest.h
    Atom.h
    AtomIndex.h
    AtomSpaceDefinitions.h
    AtomSpace.h
    AtomSpaceAsync.h
    AtomSpaceImpl.h
    AtomTable.h
    ${CMAKE_CURRENT_BINARY_DIR}/atom_types.h
    AttentionValue.h
    AttentionBank.h
    BackingStore.h
    ClassServer.h
    CompositeTruthValue.h
    CountTruthValue.h
    FixedIntegerIndex.h
    FollowLink.h
    ForeachChaseLink.h
    Foreach.h
    ForeachTwo.h
    Handle.h
    HandleMap.h
    HandleSeqIndex.h
    HandleSetIndex.h
    ImportanceIndex.h
    IncomingIndex.h
    IndefiniteTruthValue.h
    Intersect.h
    Link.h
    LinkIndex.h
    NameIndex.h
    Node.h
    NodeIndex.h
    NullTruthValue.h
    PredicateEvaluator.h
    PredicateIndex.h
    SimpleTruthValue.h
    StatisticsMonitor.h
    StringIndex.h
    TargetTypeIndex.h
    TLB.h
    Trail.h
    TruthValue.h
    TypeIndex.h
    VersionHandle.h
    types.h
    DESTINATION "include/${PROJECT_NAME}/atomspace"
)

ADD_EXECUTABLE (atomspace_async
    atomspace_async_tester
)

TARGET_LINK_LIBRARIES (atomspace_async
	atomspace
	cogutil
)

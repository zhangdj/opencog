#include "ComboProcedureRepository.h"
#include <fstream>

#include "util/Logger.h"
#include "util/StringManipulator.h"

#include <opencog/atomspace/TLB.h>

#include "PetComboVocabulary.h"
#include "NetworkElement.h"

using namespace PetCombo;
using namespace Procedure;
using namespace std;

ComboProcedureRepository::ComboProcedureRepository() {
  
}

//parse and load procedures from a stream - returns # of functions read
unsigned int ComboProcedureRepository::loadFromStream(istream& in) {
  
  bool tc = static_cast<bool>(atoi(MessagingSystem::NetworkElement::parameters.get("TYPE_CHECK_LOADING_PROCEDURES").c_str()));

  unsigned int n = 0;
  
  while (in.good()) {
    while (in.peek()==' ' || in.peek()=='\n' || in.peek()=='\t')
      in.get();
    
    if(in.peek()=='#') { //a comment line
      char tmp[LINE_CHAR_MAX];
      in.getline(tmp,LINE_CHAR_MAX);
      continue;
    }
    
    procedure_call pc = load_procedure_call<pet_builtin_action, pet_perception,
      pet_action_symbol, pet_indefinite_object>(in, false);
    
    if (!in.good()){
      break;
    }
    
    if(pc) {
      ComboProcedure* cp = new ComboProcedure(*pc);
      add(*cp);
      ++n;
      logger().log(opencog::Logger::FINE, 
		      "ComboProcedureRepository - Loaded '%s' with arity '%d'.", 
		      pc->get_name().c_str(), pc->arity());
      
    } else {
      logger().log(opencog::Logger::ERROR, 
		      "ComboProcedureRepository - Error parsing combo function.");
    }
    delete(pc);
  }
  //doing the resolution and type checking here
  //allows mutual recursion amongst functions to be defined in the input
  instantiate_procedure_calls(true);
  if(tc) {
    bool type_check_success = infer_types_repo();
    if(!type_check_success) {
      stringstream ss;
      toStream(ss, true);
      print(true);
      opencog::cassert(TRACE_INFO, type_check_success,
	      "Type Checking Error, one or more function are ill formed. See the list of function and there types :\n%s", ss.str().c_str());
    }
  }
  return n;
}

bool ComboProcedureRepository::contains(const std::string& name) const {
  return does_contain(name); 
}

const ComboProcedure& ComboProcedureRepository::get(const std::string& name) const {
  procedure_call pc=instance(name);
  opencog::cassert(TRACE_INFO, pc,
	  "No ComboProcedure matches that name, you cannot get it");
  return dynamic_cast<const ComboProcedure&>(*pc);
}

void ComboProcedureRepository::add(const ComboProcedure& cp) {
  combo::combo_tree newTr = cp.getComboTree();
  instantiateProcedureCalls(newTr, true);
  ComboProcedure* newCp = new ComboProcedure(cp.getName(),
					     cp.getArity(),
					     newTr);
  procedure_repository::add(const_cast<procedure_call_base*>(dynamic_cast<procedure_call>(newCp)));
}

void ComboProcedureRepository::instantiateProcedureCalls(combo::combo_tree& tr,bool warnOnDefiniteObj) const {
  procedure_repository::instantiate_procedure_calls(tr, warnOnDefiniteObj);
}

const char* ComboProcedureRepository::getId() const {
  return "ComboProcedureRepository";
}

void ComboProcedureRepository::saveRepository(FILE* dump) const {
  logger().log(opencog::Logger::DEBUG, "Saving %s (%ld)\n", getId(), ftell(dump));
  fprintf(dump, "%d", _repo.size());
  for (str_proc_map_const_it itr = _repo.begin(); itr != _repo.end(); itr++) {
    const string &name = itr->first; 
    int nameLength = name.length();
    fwrite(&nameLength, sizeof(int), 1, dump);
    fwrite(name.c_str(), sizeof(char), nameLength+1, dump);
    int arity = itr->second->arity();
    fwrite(&arity, sizeof(int), 1, dump);
    string treeStr = opencog::toString(itr->second->get_body());
    logger().log(opencog::Logger::FINE, "name: %s\ntree: %s\n", name.c_str(), treeStr.c_str()); 
    int treeStrLength = treeStr.length();  
    fwrite(&treeStrLength, sizeof(int), 1, dump);
    fwrite(treeStr.c_str(), sizeof(char), treeStrLength+1, dump);
  } 
}

void ComboProcedureRepository::loadRepository(FILE* dump, HandleMap<Atom *>* conv) {
  logger().log(opencog::Logger::DEBUG, "Loading %s (%ld)\n", getId(), ftell(dump));
  char buffer[1<<16];

  bool tc = static_cast<bool>(atoi(MessagingSystem::NetworkElement::parameters.get("TYPE_CHECK_LOADING_PROCEDURES").c_str()));

  
  int size;
  fscanf(dump, "%d", &size);
  for (int i = 0; i < size; i++) {
    // get the name
    int nameLength; 
    fread(&nameLength, sizeof(int), 1, dump);  
    fread(buffer, sizeof(char), nameLength+1, dump);
    string name(buffer);
    int arity;
    fread(&arity, sizeof(int), 1, dump);
    // get the combo_tree  
    int treeStrLength; 
    fread(&treeStrLength, sizeof(int), 1, dump);  
    fread(buffer, sizeof(char), treeStrLength+1, dump);
    logger().log(opencog::Logger::FINE, "name: %s\ntree: %s\n", name.c_str(), buffer); 
    std::stringstream ss(buffer);
    combo_tree tr;
    ss >> tr;
    // add the new entry
    add(ComboProcedure(name, arity, tr));
  }
  //doing the resolution here allows for mutual recursion amongst functions defined in the input
  instantiate_procedure_calls(true);
  // TODO: if combo_tree contains Handles, they must be replaced by the new ones, according with the passed handleMap

  //perform type checking
  if(tc) {
    bool type_check_success = infer_types_repo();
    if(!type_check_success) {
      stringstream ss;
      toStream(ss, true);
      print(true);
      opencog::cassert(TRACE_INFO, type_check_success,
	      "Type Checking Error, one or more function are ill formed. See the list of function and there types :\n%s", ss.str().c_str());
    }
  }
}

void ComboProcedureRepository::clear() {
  procedure_repository::clear();
}

#include <string>
using std::string;
#include <vector>
using std::vector;
#include <map>
using std::map;

#include <cassert>

#include <TLegend.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TPaveText.h>
#include <TTree.h>
#include <TError.h>
#include <TCanvas.h>
#include <TArrow.h>
#include <THStack.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TChain.h>
#include <TMath.h>
#include <TApplication.h>

#include "rootcursor.h"

/*
 * Map of attribute types.
 */

typedef struct TypeMap
{
  const char *s_atttype;
  RootAttributeType atttype;
} TypeMap;

static const TypeMap typesMap[] =
{
  { "Int_t", RootInt },
  { "UInt_t", RootUInt },
  { "Float_t", RootFloat },
  { "Bool_t", RootBool },
  { "int", RootInt },
  { "unsigned int", RootUInt },
  { "float", RootFloat },
  { "bool", RootBool },
  
  /* Collection types */
  { "vector<int>", RootInt },
  { "vector<unsigned int>", RootUInt },
  { "vector<float>", RootFloat },
  { "vector<bool>", RootBool },

  /* Sentinel */
  { NULL, RootInvalidType }
};

/*
 * Main ROOT class for handling chain of files.
 */

struct Root {
  vector<string>        files;
  map<string, TChain *> chains;
  
  Root(char *fnames[], int n)
  {
    for (int i = 0; i < n; i++)
    {
      files.push_back(fnames[i]);
    }  
  }
  
  TChain *get_chain(char *tree_name)
  {
    if (chains.find(tree_name) == chains.end())
    {
      TChain *chain = new TChain(tree_name);
      if (!chain)
      {
        return NULL;
      }
      for (vector<string>::iterator it = files.begin();
           it != files.end(); it++)
      {
        if (!chain->AddFile(it->c_str()))
        {
          delete chain;
          return NULL;
        }
      }
      chains[tree_name] = chain;
    }
    return chains[tree_name];
  }
};

/*
 * Base class for ROOT cursors.
 */

struct RootCursor {
  typedef struct Slot
  {
    RootAttributeType atttype;
    TBranch           *branch;
    void              *data;
  } Slot;

  RootTable *table;
  int nattrs;
  Slot *slots;
  
  RootCursor(RootTable *table, int nattrs)
  : table(table), nattrs(nattrs), slots(NULL) { }
  
  virtual int set(int pos, char *attname, RootAttributeType atttype) = 0;
  virtual RootAttributeType get_type(int pos) = 0;

  virtual void open() = 0;
  virtual bool next() = 0;
  virtual bool is_null(int pos) = 0;
  virtual int64_t get_tree_id(int pos) = 0;
  virtual int get_collection_id(int pos) = 0;
  virtual int get_int(int pos) = 0;
  virtual unsigned int get_uint(int pos) = 0;
  virtual float get_float(int pos) = 0;
  virtual bool get_bool(int pos) = 0;      
  virtual void close() = 0;
};

/*
 * ROOT Table.
 */

struct RootTable
{
  TChain  *chain;
  bool    is_collection;
    
  RootTable(Root *root, char *tree_name, bool is_collection)
    : is_collection(is_collection)
  {
    chain = root->get_chain(tree_name);
  }
  
  int64_t get_size()
  {
    int64_t n = chain->GetEntries();
    return (!is_collection) ? n : n * 10;
  }
};

/*
 * ROOT Table cursor.
 */

struct RootTableCursor : RootCursor
{
  int64_t chain_entry;
  int64_t chain_size;
  
  RootTableCursor(RootTable *table, int nattrs)
    : RootCursor(table, nattrs),
      chain_entry(-1), chain_size(-1) { }
  
  int
  set(int pos, char *attname, RootAttributeType atttype)
  {
    if (!slots)
    {
      slots = new Slot[nattrs];
      if (!slots)
      {
        return 0;
      }
    }
  
    slots[pos].atttype = atttype;
    slots[pos].branch = NULL;
    slots[pos].data = NULL;
    switch (atttype)
    {
    case RootTreeId:
      /* No need to reserve space for "virtual" column */
      break;
    case RootInt:
      slots[pos].data = new Int_t;
      break;
    case RootUInt:
      slots[pos].data = new UInt_t;
      break;
    case RootFloat:
      slots[pos].data = new Float_t;
      break;
    case RootBool:
      slots[pos].data = new Bool_t;
      break;
    default:
      return 0;
    }
  
    table->chain->SetBranchAddress(attname, slots[pos].data, &slots[pos].branch);
    // TODO: Validate setBranchAddress
    return 1;
  }
  
  RootAttributeType
  get_type(int pos)
  {
    return slots[pos].atttype;
  }
  
  ~RootTableCursor()
  {
    if (slots)
    {
      delete[] slots;
    }
    // TODO: Release 'data' and 'branch'
  }

  void
  open()
  {
    chain_size = table->chain->GetEntries();
  }
  
  bool
  next()
  {
    chain_entry++;
    if (chain_entry >= chain_size)
    {
      return false;
    }
    else
    {
      int i;
      int64_t local_entry = table->chain->LoadTree(chain_entry);
      for (i = 0; i < nattrs; i++)
      {
        /* Skip reading virtual columns */      
        if (slots[i].branch != NULL)
        {
          slots[i].branch->GetEntry(local_entry);
        }
      }
      return true;
    }
  }

  bool
  is_null(int pos)
  {
    return false;
  }

  int64_t
  get_tree_id(int pos)
  {
    assert(slots[pos].atttype == RootTreeId);
    return chain_entry;
  }
    
  int
  get_collection_id(int pos)
  {
    // FIXME: Redesign OO hierarchy to disallow this method
    assert(false);
  }
  
  int
  get_int(int pos)
  {
    assert(slots[pos].atttype == RootInt);
    return *((Int_t *) slots[pos].data);
  }

  unsigned int
  get_uint(int pos)
  {
    assert(slots[pos].atttype == RootUInt);
    return *((UInt_t *) slots[pos].data);
  }

  float
  get_float(int pos)
  {
    assert(slots[pos].atttype == RootFloat);  
    return *((Float_t *) slots[pos].data);
  }
  
  bool
  get_bool(int pos)
  {
    assert(slots[pos].atttype == RootBool);  
    return *((Bool_t *) slots[pos].data);
  }

  void
  close() { }
};

/*
 * ROOT Collection cursor.
 */

struct RootCollectionCursor : RootCursor
{
  int64_t chain_entry;
  int64_t chain_size;
  int     collection_entry;
  int     collection_size;

  RootCollectionCursor(RootTable *table, int nattrs)
    : RootCursor(table, nattrs),
      chain_entry(-1), chain_size(-1),
      collection_entry(-1), collection_size(-1) { }

  int
  set(int pos, char *attname, RootAttributeType atttype)
  {
    if (!slots)
    {
      slots = new Slot[nattrs];
      if (!slots)
      {
        return 0;
      }
    }
    slots[pos].atttype = atttype;
    slots[pos].data = NULL;
    slots[pos].branch = NULL;
  
    table->chain->SetBranchAddress(attname, &slots[pos].data, &slots[pos].branch);
    // TODO: Validate setBranchAddress    
    return 1;
  }
  
  RootAttributeType
  get_type(int pos)
  {
    return slots[pos].atttype;
  }

  ~RootCollectionCursor()
  {
    if (slots)
    {
      delete[] slots;
    }
    // TODO: Release 'data' and 'branch'
  }
  
  void
  open()
  {
    chain_size = table->chain->GetEntries();
  }
  
  bool
  next()
  {
    collection_entry++;
    if (collection_entry >= collection_size)
    {
      return next_collection();
    }
    return true;
  }
  
  bool
  is_null(int pos)
  {
    return false;
  }

  int64_t
  get_tree_id(int pos)
  {
    assert(slots[pos].atttype == RootTreeId);
    return chain_entry;
  }
    
  int
  get_collection_id(int pos)
  {
    assert(slots[pos].atttype == RootCollectionId);
    return collection_entry;
  }

  int
  get_int(int pos)
  {
    assert(slots[pos].atttype == RootInt);   
    return ((std::vector<int> *) slots[pos].data)->at(collection_entry);
  }

  unsigned int
  get_uint(int pos)
  {
    assert(slots[pos].atttype == RootUInt);   
    return ((std::vector<unsigned int> *) slots[pos].data)->at(collection_entry);
  }

  float
  get_float(int pos)
  {
    assert(slots[pos].atttype == RootFloat);
    return ((std::vector<float> *) slots[pos].data)->at(collection_entry);
  }

  bool
  get_bool(int pos)
  {
    assert(slots[pos].atttype == RootBool);  
    return ((std::vector<bool> *) slots[pos].data)->at(collection_entry);
  }

  void
  close() { }  

private:
  bool
  next_collection()
  {  
    while (1)
    {
      chain_entry++;
      if (chain_entry >= chain_size)
      {
        return false;
      }
      
      int64_t local_entry = table->chain->LoadTree(chain_entry);
      
      /* Find first non-virtual column */
      int first = -1;
      for (int i = 0; i < nattrs; i++)
      {
        if (slots[i].branch != NULL)
        {
          first = i;
          break;
        }
      }
      assert(first >= 0);      
    
      /* Read its collection size */
      slots[first].branch->GetEntry(local_entry);
      switch (slots[first].atttype)
      {
      case RootInt:
        collection_size = ((std::vector<int> *) slots[first].data)->size();
        break;
      case RootUInt:
        collection_size = ((std::vector<unsigned int> *) slots[first].data)->size();
        break;
      case RootFloat:
        collection_size = ((std::vector<float> *) slots[first].data)->size();
        break;
      default:
        break;
      }
      
      if (collection_size == 0)
      {
        continue;
      }
      else
      {
        for (int i = 0; i < nattrs; i++)
        {
          /* Skip reading virtual columns plus column already read */
          if (slots[i].branch != NULL && i != first)
          {
            slots[i].branch->GetEntry(local_entry);
          }
        }
        collection_entry = 0;
        return true;
      }
    }
  }
};

/*
 * C wrappers.
 */

typedef struct TableAttribute
{
	char *name;
  RootAttributeType type;
} TableAttribute;

extern "C"
RootAttributeType get_root_type(char *atttype)
{
  for (int i = 0; typesMap[i].s_atttype != NULL; i++)
  {
    if (strcmp(typesMap[i].s_atttype, atttype) == 0)
    {
      return typesMap[i].atttype;
    }
  }
  return RootInvalidType;
}

extern "C"
Root
*init_root(char *fnames[], int n)
{
  gErrorIgnoreLevel = kFatal;

  return new Root(fnames, n);
}

extern "C"
RootTable
*get_root_table(Root *root, char *tree_name, bool is_collection)
{
  return new RootTable(root, tree_name, is_collection);
}

extern "C"
void
fini_root(Root *root)
{
  delete root;
}

extern "C"
int64_t
get_root_table_approx_size(RootTable *table)
{
  return table->get_size();
}

extern "C"
void
fini_root_table(RootTable *table)
{
  delete table;
}

extern "C"
RootCursor
*init_root_cursor(RootTable *table, int nattrs)
{
  RootCursor *cursor;
  cursor = (table->is_collection) ? (RootCursor *) new RootCollectionCursor(table, nattrs)
                                  : (RootCursor *) new RootTableCursor(table, nattrs);
  return cursor;
}

extern "C"
int
set_root_cursor_attr(RootCursor *cursor, int pos, char *attname, RootAttributeType atttype)
{
  return cursor->set(pos, attname, atttype);
}

extern "C"
RootAttributeType
get_root_cursor_attr_type(RootCursor *cursor, int pos)
{
  return cursor->get_type(pos);
}

extern "C"
int
open_root_cursor(RootCursor *cursor)
{
  cursor->open();
  return 1;
}

extern "C"
int
advance_root_cursor(RootCursor *cursor)
{
  return cursor->next();
}

extern "C"
int
is_null(RootCursor *cursor, int pos)
{
  return cursor->is_null(pos);
}

extern "C"
int64_t
get_tree_id(RootCursor *cursor, int pos)
{
  return cursor->get_tree_id(pos);
}

extern "C"
int
get_collection_id(RootCursor *cursor, int pos)
{
  return cursor->get_collection_id(pos);
}

extern "C"
int
get_int(RootCursor *cursor, int pos)
{
  return cursor->get_int(pos);
}

extern "C"
unsigned int
get_uint(RootCursor *cursor, int pos)
{
  return cursor->get_uint(pos);
}

extern "C"
float
get_float(RootCursor *cursor, int pos)
{
  return cursor->get_float(pos);
}

extern "C"
bool
get_bool(RootCursor *cursor, int pos)
{
  return cursor->get_bool(pos);
}

extern "C"
void
fini_root_cursor(RootCursor *cursor)
{
  cursor->close();
  delete cursor;
}



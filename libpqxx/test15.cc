#include <iostream>

#include "pg_connection.h"
#include "pg_nontransaction.h"
#include "pg_result.h"

using namespace PGSTD;
using namespace Pg;


// Define, locally but with C linkage, a function to process warnings
// generated by the database connection.  This is optional.
namespace
{
extern "C"
{
void ReportWarning(void *, const char msg[])
{
  cerr << msg;
}
}
}


// Simple test program for libpqxx.  Open connection to database, start
// a dummy transaction to gain nontransactional access, and perform a query.
//
// Usage: test15 [connect-string]
//
// Where connect-string is a set of connection options in Postgresql's
// PQconnectdb() format, eg. "dbname=template1" to select from a database
// called template1, or "host=foo.bar.net user=smith" to connect to a
// backend running on host foo.bar.net, logging in as user smith.

class ReadTables : public Transactor
{
  Result m_Result;
public:
  typedef NonTransaction TRANSACTIONTYPE;

  ReadTables() : Transactor("ReadTables") {}

  void operator()(NonTransaction &T)
  {
    m_Result = T.Exec("SELECT * FROM pg_tables");
  }

  void OnCommit()
  {
    for (Result::const_iterator c = m_Result.begin(); c != m_Result.end(); ++c)
    {
      string N;
      c[0].to(N);

      cout << '\t' << ToString(c.num()) << '\t' << N << endl;
    }
  }
};


int main(int argc, char *argv[])
{
  try
  {
    Connection C(argv[1] ? argv[1] : "");

    C.SetNoticeProcessor(ReportWarning, 0);

    C.Perform(ReadTables());
  }
  catch (const exception &e)
  {
    // All exceptions thrown by libpqxx are derived from std::exception
    cerr << "Exception: " << e.what() << endl;
    return 2;
  }
  catch (...)
  {
    // This is really unexpected (see above)
    cerr << "Unhandled exception" << endl;
    return 100;
  }

  return 0;
}

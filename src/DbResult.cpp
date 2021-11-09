#define STRICT_R_HEADERS
#define R_NO_REMAP


#include "pch.h"
#include "DbResult.h"
#include "DbConnection.h"
#include "DbResultImpl.h"



// Construction ////////////////////////////////////////////////////////////////

DbResult::DbResult(const DbConnectionPtr& pConn) :
  pConn_(pConn)
{
  pConn_->check_connection();

  // subclass constructor can throw, the destructor will remove the
  // current result set
  pConn_->set_current_result(this);
}

DbResult::~DbResult() {
  try {
    if (is_active()) {
      pConn_->reset_current_result(this);
    }
  } catch (...) {}
}


// Publics /////////////////////////////////////////////////////////////////////

bool DbResult::complete() const {
  return (impl == NULL) || impl->complete();
}

bool DbResult::is_active() const {
  return pConn_->is_current_result(this);
}

int DbResult::n_rows_fetched() {
  return impl->n_rows_fetched();
}

int DbResult::n_rows_affected() {
  return impl->n_rows_affected();
}

void DbResult::bind(const Rcpp::List& params) {
  validate_params(params);
  impl->bind(params);
}

Rcpp::List DbResult::fetch(const int n_max) {
  if (!is_active())
    Rcpp::stop("Inactive result set");

  return impl->fetch(n_max);
}

Rcpp::List DbResult::get_column_info() {
  Rcpp::List out = impl->get_column_info();

  out.attr("row.names") = Rcpp::IntegerVector::create(NA_INTEGER, -Rf_length(out[0]));
  out.attr("class") = "data.frame";

  return out;
}

void DbResult::close() {
  // Called from destructor
  if (impl) impl->close();
}

// Privates ///////////////////////////////////////////////////////////////////

void DbResult::validate_params(const Rcpp::List& params) const {
  if (params.size() != 0) {
    SEXP first_col = params[0];
    int n = Rf_length(first_col);

    for (int j = 1; j < params.size(); ++j) {
      SEXP col = params[j];
      if (Rf_length(col) != n)
        Rcpp::stop("Parameter %i does not have length %d.", j + 1, n);
    }
  }
}

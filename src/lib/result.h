#ifndef RESULT_H
#define RESULT_H

template<typename T, typename E>
struct Result {
  static Result<T, E> Ok(T &&t) { return Result<T, E>(t); }
  static Result<T, E> Err(E &&e) { return Result<T, E>(e, tag()); }
  static Result<T, E> Ok(const T &t) { return Result<T, E>(t); }
  static Result<T, E> Err(const E &e) { return Result<T, E>(e, tag()); }

  Result(Result &&o) : k_(o.k_) {
    switch (k_) {
    case UT: new (&u_.t_) T(std::move(o.u_.t_)); break;
    case UF: new (&u_.e_) E(std::move(o.u_.e_)); break;
    default: break;
    }
    o.k_ = UD;
  }

  Result(const Result& o) : k_(o.k_) {
    switch (k_) {
    case UT: new (&u_.t_) T(o.u_.t_); break;
    case UF: new (&u_.e_) E(o.u_.e_); break;
    default: break;
    }
  }

  ~Result() {
    switch (k_) {
    case UT: u_.t_.~T(); break;
    case UF: u_.e_.~E(); break;
    default: break;
    }
  }

  bool ok() { return k_ == UT; }
  T &get() { return u_.t_; }
  E &get_err() { return u_.e_; }

private:
  struct tag {};
  Result() {}
  Result(T &&t) : k_(UT) { new (&u_.t_) T(t); }
  Result(E &&e, tag) : k_(UF) { new (&u_.e_) E(e); }
  Result(const T &t) : k_(UT) { new (&u_.t_) T(t); }
  Result(const E &e, tag) : k_(UF) { new (&u_.e_) E(e); }

  union U {
    T t_;
    E e_;
    U() {}
    ~U() {}
  };

  enum { UF, UT, UD } k_;
  U u_;
};

template<typename E>
struct Result<void, E> {
  static Result<void, E> Ok() { return Result<void, E>(); }
  static Result<void, E> Err(E &&e) { return Result<void, E>(e); }
  static Result<void, E> Err(const E &e) { return Result<void, E>(e); }

  Result(Result &&o) : k_(o.k_) {
    switch (k_) {
    case UF: new (&u_.e_) E(std::move(o.u_.e_)); break;
    default: break;
    }
    o.k_ = UD;
  }

  Result(const Result& o) : k_(o.k_) {
    switch (k_) {
    case UF: new (&u_.e_) E(o.u_.e_); break;
    default: break;
    }
  }

  ~Result() {
    switch (k_) {
    case UF: u_.e_.~E(); break;
    default: break;
    }
  }

  bool ok() { return k_ == UT; }
  E &get_err() { return u_.e_; }

private:
  Result() : k_(UT) {}
  Result(E &&e) : k_(UF) { new (&u_.e_) E(e); }
  Result(const E &e) : k_(UF) { new (&u_.e_) E(e); }

  union U {
    E e_;
    U() {}
    ~U() {}
  };

  enum { UF, UT, UD } k_;
  U u_;
};

#endif /* RESULT_H */

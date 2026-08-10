// Test program to detect JANA2 v2.4.3 Input<T> bug
// The bug is fixed in commit f18b66c6
//
// Bug: Input<T> constructor sets m_databundle_name = m_type_name
//      but GetCollection() uses m_tag (which is never set)
// Fix: Removes m_tag member, uses m_databundle_name everywhere

#include <JANA/Components/JHasInputs.h>

// Helper class that can access protected Input<T>
struct TestHasInputs : public jana::components::JHasInputs {
  static bool has_bug() {
    TestHasInputs owner;
    Input<int> test_input(&owner);

    // In buggy version: constructor sets m_databundle_name = "int"
    // In fixed version: constructor doesn't set m_databundle_name (empty)

    return test_input.GetDatabundleName() == "int";
  }
};

int main() {
  // Return 1 if bug detected, 0 if fixed
  return TestHasInputs::has_bug() ? 1 : 0;
}

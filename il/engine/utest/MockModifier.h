#include "AbstModifier.h"

USING_L4L7_ENGINE_NS;

///////////////////////////////////////////////////////////////////////////////

class MockModifier : public AbstModifierCrtp<MockModifier>
{
  public:
    MockModifier() : mLastCursor(0) {}

    void SetCursor(int32_t posIndex)
    {
        mLastCursor = posIndex; 
    }

    void Next()
    {
        ++mLastCursor;
    }
    
    void GetValue(uint8_t * buffer, bool reverse = false) const {}

    size_t GetSize() const { return 0; }

    // Mock methods

    int32_t GetCursor() { return mLastCursor; }

  private:
    int32_t mLastCursor;
};


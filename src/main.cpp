#include <iostream>
#include "memory_pool/UniquePtr.h"
#include "memory_pool/SharedPtr.h"
#include "memory_pool/PoolDeleter.h"

struct TestObject {
    int id;
    std::string name;

    TestObject(int i, const std::string& n) : id(i), name(n) {
        std::cout << "TestObject constructed: " << name << std::endl;
    }

    ~TestObject() {
        std::cout << "TestObject destroyed: " << name << std::endl;
    }

    void sayHello() {
        std::cout << "Hello from " << name << " (id=" << id << ")" << std::endl;
    }
};

int main() {
    std::cout << "=== Test 1: Basic Construction and Destruction ===" << std::endl;
    {
        unique_ptr<TestObject> ptr(new TestObject(1, "Object1"));
        ptr->sayHello();
        (*ptr).sayHello();
        std::cout << "Raw pointer: " << ptr.get() << std::endl;
    }

    std::cout << "\n=== Test 2: Boolean Conversion ===" << std::endl;
    {
        unique_ptr<TestObject> ptr1(new TestObject(2, "Object2"));
        unique_ptr<TestObject> ptr2(nullptr);

        if (ptr1) {
            std::cout << "ptr1 is valid" << std::endl;
        }
        if (!ptr2) {
            std::cout << "ptr2 is null" << std::endl;
        }
    }

    std::cout << "\n=== Test 3: Copy Should NOT Compile ===" << std::endl;
    std::cout << "(Copy tests commented out - uncomment to verify compile error)" << std::endl;

    std::cout << "\n=== Test 4: Move Semantics ===" << std::endl;
    {
        unique_ptr<TestObject> ptr1(new TestObject(10, "Movable"));
        std::cout << "ptr1 address: " << ptr1.get() << std::endl;

        unique_ptr<TestObject> ptr2(std::move(ptr1));
        std::cout << "After move constructor:" << std::endl;
        std::cout << "  ptr1 address: " << ptr1.get() << " (should be null)" << std::endl;
        std::cout << "  ptr2 address: " << ptr2.get() << " (should have the object)" << std::endl;

        if (!ptr1 && ptr2) {
            std::cout << "SUCCESS: Ownership transferred correctly!" << std::endl;
        }

        unique_ptr<TestObject> ptr3(new TestObject(20, "Another"));
        std::cout << "\nBefore move assignment:" << std::endl;
        std::cout << "  ptr3 address: " << ptr3.get() << std::endl;

        ptr3 = std::move(ptr2);

        std::cout << "After move assignment:" << std::endl;
        std::cout << "  ptr2 address: " << ptr2.get() << " (should be null)" << std::endl;
        std::cout << "  ptr3 address: " << ptr3.get() << " (should have Movable)" << std::endl;
    }

    std::cout << "\n=== Test 5: release(), reset(), swap() ===" << std::endl;
    {
        unique_ptr<TestObject> ptr1(new TestObject(30, "Release"));

        TestObject* raw = ptr1.release();
        std::cout << "After release: ptr1 is " << (ptr1 ? "valid" : "null") << std::endl;
        delete raw;

        unique_ptr<TestObject> ptr2(new TestObject(40, "Original"));
        ptr2.reset(new TestObject(50, "Replacement"));
        std::cout << "After reset: ptr2 points to " << ptr2->name << std::endl;

        unique_ptr<TestObject> ptrA(new TestObject(60, "A"));
        unique_ptr<TestObject> ptrB(new TestObject(70, "B"));
        std::cout << "Before swap: ptrA=" << ptrA->name << ", ptrB=" << ptrB->name << std::endl;
        ptrA.swap(ptrB);
        std::cout << "After swap: ptrA=" << ptrA->name << ", ptrB=" << ptrB->name << std::endl;
    }

    std::cout << "\n=== Test 6: unique_ptr with PoolAllocator ===" << std::endl;
    {
        PoolAllocator pool(sizeof(TestObject), 3);

        std::cout << "Creating objects from pool:" << std::endl;

        auto ptr1 = make_unique_from_pool<TestObject>(pool, 100, "PoolObject1");
        auto ptr2 = make_unique_from_pool<TestObject>(pool, 200, "PoolObject2");
        auto ptr3 = make_unique_from_pool<TestObject>(pool, 300, "PoolObject3");

        std::cout << "\nAll 3 objects created. Pool should be exhausted." << std::endl;

        auto ptr4 = make_unique_from_pool<TestObject>(pool, 400, "PoolObject4");
        if (!ptr4) {
            std::cout << "SUCCESS: ptr4 is null (pool exhausted)" << std::endl;
        }

        std::cout << "\nAccessing objects:" << std::endl;
        ptr1->sayHello();
        ptr2->sayHello();
        ptr3->sayHello();

        std::cout << "\nReleasing ptr2 (should return to pool):" << std::endl;
        ptr2.reset();

        std::cout << "\nNow we can allocate again:" << std::endl;
        auto ptr5 = make_unique_from_pool<TestObject>(pool, 500, "PoolObject5");
        if (ptr5) {
            std::cout << "SUCCESS: ptr5 allocated from recycled memory" << std::endl;
            ptr5->sayHello();
        }

        std::cout << "\nLeaving scope - all objects will be destroyed and returned to pool:" << std::endl;
    }
    std::cout << "Pool test complete!" << std::endl;

    std::cout << "\n=== Test 7: shared_ptr Basic Construction ===" << std::endl;
    {
        shared_ptr<TestObject> sp1(new TestObject(1000, "SharedObj1"));
        std::cout << "use_count: " << sp1.use_count() << " (expected 1)" << std::endl;
        sp1->sayHello();
        (*sp1).sayHello();
        std::cout << "Raw pointer: " << sp1.get() << std::endl;
    }

    std::cout << "\n=== Test 8: shared_ptr Copy Semantics ===" << std::endl;
    {
        shared_ptr<TestObject> sp1(new TestObject(2000, "CopyShared"));
        std::cout << "sp1 use_count: " << sp1.use_count() << " (expected 1)" << std::endl;

        shared_ptr<TestObject> sp2(sp1);
        std::cout << "After copy constructor:" << std::endl;
        std::cout << "  sp1 use_count: " << sp1.use_count() << " (expected 2)" << std::endl;
        std::cout << "  sp2 use_count: " << sp2.use_count() << " (expected 2)" << std::endl;
        std::cout << "  Same object? " << (sp1.get() == sp2.get() ? "YES" : "NO") << std::endl;

        shared_ptr<TestObject> sp3(new TestObject(2001, "AnotherShared"));
        std::cout << "\nBefore copy assignment: sp3 use_count: " << sp3.use_count() << std::endl;
        sp3 = sp1;
        std::cout << "After copy assignment:" << std::endl;
        std::cout << "  sp1 use_count: " << sp1.use_count() << " (expected 3)" << std::endl;
        std::cout << "  sp3 use_count: " << sp3.use_count() << " (expected 3)" << std::endl;

        std::cout << "\nLeaving scope - sp1, sp2, sp3 all share the same object:" << std::endl;
    }
    std::cout << "Object should be destroyed exactly once above." << std::endl;

    std::cout << "\n=== Test 9: shared_ptr Move Semantics ===" << std::endl;
    {
        shared_ptr<TestObject> sp1(new TestObject(3000, "MoveShared"));
        std::cout << "sp1 use_count: " << sp1.use_count() << " (expected 1)" << std::endl;

        shared_ptr<TestObject> sp2(std::move(sp1));
        std::cout << "After move constructor:" << std::endl;
        std::cout << "  sp1 is " << (sp1 ? "valid" : "null") << " (expected null)" << std::endl;
        std::cout << "  sp1 use_count: " << sp1.use_count() << " (expected 0)" << std::endl;
        std::cout << "  sp2 use_count: " << sp2.use_count() << " (expected 1)" << std::endl;

        shared_ptr<TestObject> sp3(new TestObject(3001, "MoveTarget"));
        sp3 = std::move(sp2);
        std::cout << "After move assignment:" << std::endl;
        std::cout << "  sp2 is " << (sp2 ? "valid" : "null") << " (expected null)" << std::endl;
        std::cout << "  sp3 use_count: " << sp3.use_count() << " (expected 1)" << std::endl;
        sp3->sayHello();
    }

    std::cout << "\n=== Test 10: shared_ptr Boolean Conversion ===" << std::endl;
    {
        shared_ptr<TestObject> sp1(new TestObject(4000, "BoolTest"));
        shared_ptr<TestObject> sp2(nullptr);

        if (sp1) std::cout << "sp1 is valid (expected)" << std::endl;
        if (!sp2) std::cout << "sp2 is null (expected)" << std::endl;
    }

    std::cout << "\n=== Test 11: shared_ptr reset() and swap() ===" << std::endl;
    {
        shared_ptr<TestObject> sp1(new TestObject(5000, "ResetMe"));
        shared_ptr<TestObject> sp2(sp1);
        std::cout << "Before reset: use_count = " << sp1.use_count() << " (expected 2)" << std::endl;

        sp1.reset();
        std::cout << "After sp1.reset():" << std::endl;
        std::cout << "  sp1 is " << (sp1 ? "valid" : "null") << " (expected null)" << std::endl;
        std::cout << "  sp2 use_count: " << sp2.use_count() << " (expected 1)" << std::endl;

        sp1.reset(new TestObject(5001, "Replacement"));
        std::cout << "After sp1.reset(new obj): sp1 use_count = " << sp1.use_count() << std::endl;

        shared_ptr<TestObject> spA(new TestObject(6000, "SwapA"));
        shared_ptr<TestObject> spB(new TestObject(6001, "SwapB"));
        std::cout << "\nBefore swap: spA=" << spA->name << ", spB=" << spB->name << std::endl;
        spA.swap(spB);
        std::cout << "After swap:  spA=" << spA->name << ", spB=" << spB->name << std::endl;
    }

    std::cout << "\n=== Test 12: shared_ptr Reference Count Lifecycle ===" << std::endl;
    {
        shared_ptr<TestObject> sp1(new TestObject(7000, "Lifecycle"));
        std::cout << "Created sp1, use_count = " << sp1.use_count() << std::endl;
        {
            shared_ptr<TestObject> sp2(sp1);
            std::cout << "Created sp2 (copy), use_count = " << sp1.use_count() << std::endl;
            {
                shared_ptr<TestObject> sp3(sp1);
                std::cout << "Created sp3 (copy), use_count = " << sp1.use_count() << std::endl;
            }
            std::cout << "sp3 destroyed, use_count = " << sp1.use_count() << std::endl;
        }
        std::cout << "sp2 destroyed, use_count = " << sp1.use_count() << std::endl;
    }
    std::cout << "sp1 destroyed - object should be deleted above." << std::endl;

    std::cout << "\n=== Test 13: shared_ptr with PoolAllocator ===" << std::endl;
    {
        PoolAllocator pool(sizeof(TestObject), 3);

        std::cout << "Creating shared objects from pool:" << std::endl;
        auto sp1 = make_shared_from_pool<TestObject>(pool, 8000, "PoolShared1");
        auto sp2 = make_shared_from_pool<TestObject>(pool, 8001, "PoolShared2");
        auto sp3 = make_shared_from_pool<TestObject>(pool, 8002, "PoolShared3");

        std::cout << "Pool should be exhausted." << std::endl;
        auto sp4 = make_shared_from_pool<TestObject>(pool, 8003, "PoolShared4");
        if (!sp4) {
            std::cout << "SUCCESS: sp4 is null (pool exhausted)" << std::endl;
        }

        std::cout << "\nTesting shared ownership with pool:" << std::endl;
        {
            shared_ptr<TestObject> sp1_copy(sp1);
            std::cout << "sp1 use_count after copy: " << sp1.use_count() << " (expected 2)" << std::endl;
        }
        std::cout << "sp1 use_count after copy destroyed: " << sp1.use_count() << " (expected 1)" << std::endl;

        std::cout << "\nResetting sp2 (returns memory to pool):" << std::endl;
        sp2.reset();

        auto sp5 = make_shared_from_pool<TestObject>(pool, 8004, "PoolShared5");
        if (sp5) {
            std::cout << "SUCCESS: sp5 allocated from recycled pool memory" << std::endl;
            sp5->sayHello();
        }

        std::cout << "\nLeaving scope - all pool objects will be destroyed:" << std::endl;
    }
    std::cout << "Pool shared_ptr test complete!" << std::endl;

    std::cout << "\n=== All Tests Complete ===" << std::endl;
    return 0;
}

void abs_cls_main();


class ConnectableBase {
public:
    ConnectableBase(const std::string& aName);
    ~ConnectableBase();
    std::string mName;
};

// Base class using CRTP (curiously reocurring template pattern)
template <typename Derived>
class Connectable: public ConnectableBase {
public:
    // Makes connectImpl "pure virtual" - has to be defined in derived class
    Connectable(const std::string& aName);

    // Makes disconnectImpl "pure virtual" - has to be defined in derived class
    ~Connectable();

    // Prevent copying to avoid confusion with connect/disconnect
    Connectable(const Connectable&) = delete;
    Connectable& operator=(const Connectable&) = delete;
};

// Derived class 1
class DeviceA : public Connectable<DeviceA> {
public:
    DeviceA(const std::string& aName);
    ~DeviceA();
    void connectImpl();

    void disconnectImpl();

    int getValue() const { return mValue; }

    int mValue = 1; //? Define before implementation?

};

// Derived class 2
class DeviceB : public Connectable<DeviceB> {
public:
    DeviceB(const std::string& aName);
    ~DeviceB();

    void connectImpl();

    void disconnectImpl();

    int mValue = 2;
};

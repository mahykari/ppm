#ifndef MONITORABLE_SYSTEM_HH
#define MONITORABLE_SYSTEM_HH

class MonitorableSystem {
public:
  virtual void next() = 0;
  virtual const std::vector<bool>& data() = 0;
};

#endif

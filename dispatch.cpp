#include <memory>
#include <cstdint>

class TensorImpl {
  int64_t size_;
  std::unique_ptr<float[]> data_;
public:
  TensorImpl(int64_t size) : size_(size) {
    data_ = std::make_unique<float[]>(size);
  }
  int64_t numel() const {
    return size_;
  }
  float* data() const {
    return data_.get();
  }
};

using Tensor = std::shared_ptr<TensorImpl>;

namespace native {

  Tensor negate(const Tensor& t) {
    auto r = std::make_shared<TensorImpl>(t->numel());
    auto* p_t = t->data();
    auto* p_r = r->data();
    for (int64_t i = 0; i < t->numel(); i++) {
      p_r[i] = -p_t[i];
    }
    return r;
  }

  Tensor zeros(int64_t numel) {
    auto r = std::make_shared<TensorImpl>(numel);
    auto* p_r = r->data();
    for (int64_t i = 0; i < numel; i++) {
      p_r[i] = 0;
    }
    return r;
  }

}

void* negate_ptr;
void* zeros_ptr;

#if 1
Tensor zeros(int64_t numel) {
  return (*(Tensor(*)(int64_t))zeros_ptr)(numel);
}

Tensor negate(const Tensor& t) {
  return (*(Tensor(*)(const Tensor&))negate_ptr)(t);
}
#else
Tensor zeros(int64_t numel) {
  return native::zeros(numel);
}

Tensor negate(const Tensor& t) {
  return native::negate(t);
}
#endif

int main() {
  // initialization
  negate_ptr = (void*)&native::negate;
  zeros_ptr= (void*)&native::zeros;

  // work
  auto t = zeros(1);
  auto r = negate(t);
  return 0;
}

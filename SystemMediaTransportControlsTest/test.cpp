#include "public.h"

//#include <winrt/Base.h>
#include <Windows.Foundation.h>
#include <wrl\wrappers\corewrappers.h>
#include <wrl\client.h>
#include <wrl/event.h>

#include <comdef.h>

extern "C" {

HRESULT impl_ButtonPressed(
	ABI::Windows::Media::ISystemMediaTransportControls *sender,
	ABI::Windows::Media::ISystemMediaTransportControlsButtonPressedEventArgs *args
) {
	return S_OK;
}

LRESULT test(void) {
	using namespace Microsoft::WRL::Wrappers;
	using namespace Microsoft::WRL;
	HRESULT nRet;
	void *p;

	using ABI::Windows::Media::ISystemMediaTransportControls;
	using ABI::Windows::Media::ISystemMediaTransportControlsButtonPressedEventArgs;
	using ABI::Windows::Media::SystemMediaTransportControls;
	using ABI::Windows::Media::SystemMediaTransportControlsButton;
	using ABI::Windows::Media::SystemMediaTransportControlsButtonPressedEventArgs;
	using ABI::Windows::Storage::Streams::IDataWriter;
	using ABI::Windows::Storage::Streams::IDataWriterFactory;
	using ABI::Windows::Storage::Streams::IOutputStream;
	using ABI::Windows::Storage::Streams::IRandomAccessStream;
	using ABI::Windows::Storage::Streams::IRandomAccessStreamReference;
	using ABI::Windows::Storage::Streams::IRandomAccessStreamReferenceStatics;

	HStringReference hstrClass(RuntimeClass_Windows_Media_SystemMediaTransportControls);

	//_com_error a(0);

	//nRet = RoInitialize(RO_INIT_MULTITHREADED);

	//if (FAILED(nRet))
	//	return nRet;

	Microsoft::WRL::ComPtr<ISystemMediaTransportControlsInterop> pCom;

	//nRet = RoGetActivationFactory(hstrClass.Get(), IID_ISystemMediaTransportControlsInterop, (void**)pCom.GetAddressOf());
	nRet = RoGetActivationFactory(hstrClass.Get(), IID_ISystemMediaTransportControlsInterop, (void**)pCom.GetAddressOf());

	//SystemMediaTransportControls
	ComPtr<ABI::Windows::Media::ISystemMediaTransportControls> temp1;
	//ABI::Windows::Foundation::IEventHandler<ABI::Windows::Media::ISystemMediaTransportControlsButtonPressedEventArgs*> temp3;
	//__FITypedEventHandler_2_Windows__CMedia__CSystemMediaTransportControls_Windows__CMedia__CSystemMediaTransportControlsButtonPressedEventArgs temp2;
	//ABI::Windows::Media::ISystemMediaTransportControlsStatics temp4;
	//ABI::Windows::Media:ISystemMediaTransportControlsStatics temp4;
	//temp1->add_ButtonPressed()

	//See https://github.com/chromium/chromium/blob/master/ui/base/win/system_media_controls/system_media_controls_service_impl.cc

	auto handler =
		Microsoft::WRL::Callback<ABI::Windows::Foundation::ITypedEventHandler<
		SystemMediaTransportControls*,
		SystemMediaTransportControlsButtonPressedEventArgs*>>(
			impl_ButtonPressed);
	//handler.
	//handler->Invoke();

	//ABI::Windows::Media::SystemMediaTransportControls

	//nRet = Windows::Foundation::GetActivationFactory(hstrClass.Get(), )

	//RoInitializeWrapper roinitwrapper(RO_INIT_SINGLETHREADED);

	/*

	//ABI::Windows::Media::SystemMediaTransportControls clsTemp;
	Microsoft::WRL::ComPtr<IInspectable> inspectable;

	auto nRet = Windows::Foundation::ActivateInstance(
		//HStringReference(RuntimeClass_Windows_Media_SystemMediaTransportControls).Get(),
		HStringReference(RuntimeClass_Windows_Media_SystemMediaTransportControls).Get(),
		inspectable.GetAddressOf()
	);

	*/

	//auto nRet = RoActivateInstance(HStringReference(RuntimeClass_Windows_Media_SystemMediaTransportControls).Get(), (IInspectable**)&p);

	//hStrRef = WindowsCreateStringReference(RuntimeClass_Windows_Media_MediaControl);
	//RoActivateInstance(hStrRef, NULL);
	//WindowsDeleteString(hStrRef);

	return nRet;
}

}
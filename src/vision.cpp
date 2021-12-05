#include "vision.h"

void addNetTableVar(bool& var, const wpi::Twine& name, std::shared_ptr<nt::NetworkTable> table) {
	if(!table->ContainsKey(name)) {
		table->PutBoolean(name.str(), var);
	} else {}
	table->GetEntry(name).AddListener(
		[&var](const nt::EntryNotification& event){
			if(event.value->IsBoolean()) {
				var = event.value->GetBoolean();
				//std::cout << " Networktable var(bool) updated to : " << var << newline;
			}
		},
		NT_NOTIFY_IMMEDIATE | NT_NOTIFY_NEW | NT_NOTIFY_UPDATE
	);
}
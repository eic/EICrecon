<!-- Vue 2.x  -->
<script>
  new Vue({
      el: '#example_vue',
    // Options...
      methods: {
            multiple: function (event) {
              this.hide()
              this.show()
            },
            hide:  function (event) { },
            show: function (event) { }
      },
      data() {
          return {
              // flag_name: true,
              // flag_value: false,
              flags: [],
              recoPrefixes: [],
              isHidden:false,
              isHiddenDefaultValue: false,
              isHiddenValue: false,
              isHiddenDescription: false,
              showDiff: false,
              showRecoOnly: false
          };
      },
      watch: {
          showDiff(value) {
              console.log(value);
              if(value) {
                  this.isHiddenDefaultValue = true;
                  this.isHiddenValue = true;
              }
          }
      },

      created() {
          // Read json flags
          fetch('arches_flags.json')
              .then(response => response.json())
              .then(data => (this.flags = data))
              .catch(err => console.log(err));
          // Define reconstruction flags prefixes
          this.recoPrefixes = [
              "B0ECAL",
              "B0TRK",
              "BEMC",
              "BTOF",
              "BTRK",
              "BVTX",
              "ECGEM",
              "ECTOF",
              "ECTRK",
              "EEMC",
              "FOFFMTRK",
              "HCAL",
              "MPGD",
              "RICH",
              "RPOTS",
              "ZDC",
              "Tracking",
              "Reco",
              "Digi",
              "Calorimetry"
          ];
      },
      computed: {
          filteredFlags() {
              let prefixes = this.recoPrefixes;
               return this.flags.filter(el => {
                   if(el[0] === "podio:output_include_collections") {
                       return false; // Zhudko krivoy kostil
                   }

                   if(this.showDiff) {
                       if(el[1].toUpperCase() === el[2].toUpperCase()) {
                           return false;
                       }
                   }

                   if(!this.showRecoOnly) {
                       return true;   // We are not filtering and return everything
                   }
                   /*
                                                    for r in all_records
                              if r[0].casefold().startswith(reco_prefix.lower())
                              and 'LogLevel' not in r[0]
                              and 'InputTags' not in r[0]
                              and 'input_tags' not in r[0]
                              and 'verbose' not in r[0]]
                        */


                   for (let prefix of prefixes) {
                       if (el[0].toUpperCase().startsWith(prefix.toUpperCase())) {
                           return true;
                       }
                   }
                   return false;
               });
          },
      },
  });
</script>

<div id="example_vue">
    <div class="radio_btn_name">
        <div>Reconstruction only</div>
        <div>Show default value</div>
        <div>User value</div>
        <div>Show description</div>
        <div>Show diff only</div>
    </div>
    <div class="radio_btn">
        <div class="toggleWrapper">
            <input type="checkbox" name="toggle1" class="mobileToggle" id="toggle1" v-model="showRecoOnly">
            <label for="toggle1"></label>
        </div>
        <div class="toggleWrapper">
            <input type="checkbox" name="toggle2" class="mobileToggle" id="toggle2" v-model="isHiddenDefaultValue">
            <label for="toggle2"></label>
        </div>
        <div class="toggleWrapper">
            <input type="checkbox" name="toggle3" class="mobileToggle" id="toggle3" v-model="isHiddenValue">
            <label for="toggle3"></label>
        </div>
        <div class="toggleWrapper">
            <input type="checkbox" name="toggle4" class="mobileToggle" id="toggle4" v-model="isHiddenDescription">
            <label for="toggle4"></label>
        </div>
        <div class="toggleWrapper">
            <input type="checkbox" name="toggle5" class="mobileToggle" id="toggle5" v-model="showDiff">
            <label for="toggle5"></label>
        </div>
    </div>
    <input type="text" id="myInput" onkeyup="filterTableRowsByInput('myInput', ['table_flags'])" placeholder="Search for flags..">
    <table class="table_flags">
        <thead>
            <tr>
                <th v-if="!isHidden">Flag name</th>
                <th v-if="isHiddenDefaultValue">Default value</th>
                <th v-if="isHiddenValue">User value</th>
                <th v-if="isHiddenDescription">Description</th>
            </tr>
        </thead>
        <tbody>
            <tr v-for="flag in filteredFlags" v-bind:title="flag[3]">
                <td >{{ flag[0] }}</td>
                <td v-if="isHiddenDefaultValue" v-bind:title="flag[1]">{{ flag[1] }}</td>
                <td v-if="isHiddenValue">{{ flag[2] }}</td>
                <td v-if="isHiddenDescription">{{ flag[3] }}</td>
            </tr>
        </tbody>
    </table>
</div>



